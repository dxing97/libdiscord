#include <stdlib.h>
//#include "libwebsockets/lib/libwebsockets.h"

#include "libdiscord.h"



static struct lws_protocols protocols[] = {
        {
                "DiscordBot",
                ld_lws_callback,
                4096,
                4096, //rx buffer size
        },
        {
                NULL, NULL, 0 //null terminator
        }
};

static const struct lws_extension exts[] = { //default lws extension, code from lws test-app/test-echo.c
        {
                "permessage-deflate",
                lws_extension_callback_pm_deflate,
                "permessage-deflate; client_no_context_takeover; client_max_window_bits"
        },
        { NULL, NULL, NULL /* terminator */ }
};

struct ld_context *ld_create_context_via_info(struct ld_context_info *info) {
    //assuming the values passed in are good
    struct ld_context *context;
    context = malloc(sizeof(struct ld_context));

    context->log_level = info->log_level;

    context->gateway_state = LD_GATEWAY_UNCONNECTED;

    context->user_callback = info->user_callback;

    /* curl init */
    curl_global_init(CURL_GLOBAL_DEFAULT);
    context->curl_multi_handle=curl_multi_init();

    if(info->bot_token == NULL) {
        return NULL;
    }
    context->bot_token = malloc(strlen(info->bot_token) + 1);
    context->bot_token = strcpy(context->bot_token, info->bot_token);

    lws_set_log_level(15, NULL);

    return context;
}

void ld_destroy_context(struct ld_context *context) {
    curl_multi_cleanup(context->curl_multi_handle);
    curl_global_cleanup();
    free(context);
}

void ld_err(struct ld_context *context, const char *message, ...) {
    if((ld_log_error & context->log_level) != 0) {
        va_list myargs;
        va_start(myargs, message);
        _ld_log(ld_log_error, context->log_level, message, myargs);
        va_end(myargs);
    }
}

void ld_warn(struct ld_context *context, const char *message, ...) {
    if((ld_log_warning & context->log_level) != 0) {
        va_list myargs;
        va_start(myargs, message);
        _ld_log(ld_log_warning, context->log_level, message, myargs);
        va_end(myargs);
    }
}
void ld_info(struct ld_context *context, const char *message, ...) {
    if((ld_log_info & context->log_level) != 0) {
        va_list myargs;
        va_start(myargs, message);
        _ld_log(ld_log_info, context->log_level, message, myargs);
        va_end(myargs);
    }
}
void ld_notice(struct ld_context *context, const char *message, ...) {
    if((ld_log_notice & context->log_level) != 0) {
        va_list myargs;
        va_start(myargs, message);
        _ld_log(ld_log_notice, context->log_level, message, myargs);
        va_end(myargs);
    }
}
void ld_debug(struct ld_context *context, const char *message, ...) {\
    if((ld_log_debug & context->log_level) != 0) {\
        va_list myargs;
        va_start(myargs, message);
        _ld_log(ld_log_debug, context->log_level, message, myargs);
        va_end(myargs);
    }
}

int ld_gateway_connection_state(struct ld_context *context) {
    return context->gateway_state;
}

struct _ld_buffer {
    char *string;
    size_t size;
    struct ld_context *context;
};

size_t _ld_curl_response_string(void *contents, size_t size, size_t nmemb, void *userptr){
    size_t recieved_size = size * nmemb;
    struct _ld_buffer *buffer = (struct _ld_buffer *) userptr;

    buffer->string = realloc(buffer->string, buffer->size + recieved_size + 1);
    if(buffer->string == NULL) {
        ld_err(buffer->context,
               "realloc: couldn't allocate memory for curl response string in ld_connect!");
    }

    memcpy(&(buffer->string[buffer->size]), contents, recieved_size);
    buffer->size += recieved_size;
    buffer->string[buffer->size] = '\0';

    return recieved_size;
}



int ld_connect(struct ld_context *context) {
    int ret;

    /*
     * check to see if we can even connect to Discord's servers
     * examine /gateway and see if we get a valid response
     */
    CURL *handle;
    struct _ld_buffer buffer;

    buffer.string = malloc(1);
    buffer.size = 0;
    buffer.context = context;
    handle = curl_easy_init();
    if(handle == NULL) {
        //something went wrong trying to create the easy handle.
        ld_err(context, "curl: couldn't init easy handle");
        return 2;
    }

    curl_easy_setopt(handle, CURLOPT_URL, LD_API_URL LD_REST_API_VERSION "/gateway");
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, _ld_curl_response_string);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void *)&buffer);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "DiscordBot (https://github.com/dxing97/libdiscord 0.3) ");

    ret = curl_easy_perform(handle);

    if(ret != CURLE_OK) {
        ld_err(context, "curl: couldn't get gateway url from /gateway");
        return 2;
    }

    ld_debug(context, "received data from /gateway: \n%s", buffer.string);

    //use jansson to extract the JSON data
    json_t *object, *tmp;
    json_error_t error;

    object = json_loads(buffer.string, 0, &error);
    if(object == NULL) {
        ld_err(context, "jansson: couldn't decode string returned "
                "from /gateway in ld_connext: %s", buffer.string);
        return 3;
    }

    tmp = json_object_get(object, "url");
    if(tmp == NULL) {
        ld_err(context, "jansson: couldn't find key \"url\" in JSON object from /gateway");
        return 3;
    }

    if(json_string_value(tmp) == NULL) {
        ld_err(context, "jansson: didn't receive string object from "
                "JSON payload received from gateway");
        return 3;
    }

    context->gateway_url = malloc(strlen(json_string_value(tmp)) + 1);
    context->gateway_url = strcpy(context->gateway_url, json_string_value(tmp));

    free(tmp);
    free(object);
    /*
     * we got a valid response from the REST API, which should mean
     *  Discord is connectable at basic level
     *  Now we should check the bot token validity using /gateway/bot
     */
    struct curl_slist *headers = NULL;
    char auth_header[1024];
    sprintf(auth_header, "Authorization: %s", context->bot_token);
    headers = curl_slist_append(headers, auth_header);

    //check the bot token's validity by trying to connect to /gateway/bot

    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(handle, CURLOPT_URL, LD_API_URL LD_REST_API_VERSION "/gateway/bot");

    free(buffer.string);
    buffer.string = malloc(1);
    buffer.size = 0;

    ret = curl_easy_perform(handle);
    if(ret != CURLE_OK) {
        ld_err(context, "curl: couldn't get gateway url from /gateway");
        return 2;
    }
    curl_easy_cleanup(handle);

    ld_debug(context, "received data from /gateway/bot: \n%s", buffer.string);

    object = json_loads(buffer.string, 0, &error);
    if(object == NULL) {
        ld_err(context, "jansson: couldn't decode string returned "
                "from /gateway/bot in ld_connect: %s", buffer.string);
        return 3;
    }

    tmp = json_object_get(object, "url");
    if(tmp == NULL) {
        ld_err(context, "jansson: couldn't find key \"url\" in JSON object from /gateway/bot."
                " is the bot token valid? are we being ratelimited?");
        return 3;
    }

    if(json_string_value(tmp) == NULL) {
        ld_err(context, "jansson: didn't receive string object in \"url\" from "
                "JSON payload received from /gateway/bot");
        return 3;
    }

    context->gateway_bot_url = malloc(strlen(json_string_value(tmp)) + 1);
    context->gateway_bot_url = strcpy(context->gateway_url, json_string_value(tmp));

    tmp = json_object_get(object, "shards");
    if(tmp == NULL) {
        ld_err(context, "jansson: couldn't find key \"shards\" in JSON object from /gateway/bot."
                "is the bot token valid?");
        return 3;
    }

    if(json_integer_value(tmp) == 0) {
        ld_err(context, "jansson: didn't receive integer object in \"shards\" from "
                "JSON payload received from /gateway/bot");
        return 3;
    }

    context->shards = (int) json_integer_value(tmp);
    ld_info(context, "shards: %d", context->shards);

    switch(context->gateway_state) {
        case LD_GATEWAY_UNCONNECTED:
            //we were never connected, so we should start a fresh connection
            context->gateway_state = LD_GATEWAY_CONNECTING;
            ret = ld_gateway_connect(context);
            if(ret != 0){
                return 1;
            }
            break;
        case LD_GATEWAY_DISCONNECTED:
            //we were disconnected from the gateway.
            context->gateway_state = LD_GATEWAY_CONNECTING;
            ld_gateway_resume(context);
            break;
        case LD_GATEWAY_CONNECTING:
            //???
            break;
        case LD_GATEWAY_CONNECTED:
            //this context already has an established connection to the gateway
            break;
        default:
            //???
            break;
    }
    //we're already connected...
    return 0;
}

int ld_service(struct ld_context *context) {
    /*
     * HTTP servicing
     */
//    ld_debug(context, "servicing REST requests");
    /*
     * gateway servicing
     * if sufficient time has passed, add a heartbeat payload to the queue
     */
    ld_debug(context, "servicing gateway payloads");
    lws_service(context->lws_context, 20);
    return 0;
}

int ld_gateway_connect(struct ld_context *context) {
    //lws context creation info
    struct lws_context_creation_info info;
    struct lws_context *lws_context;
    struct lws_client_connect_info *i;

    memset(&info, 0, sizeof(info));
    i = malloc(sizeof(struct lws_client_connect_info));
    memset(i, 0, sizeof(struct lws_client_connect_info));

    info.port = CONTEXT_PORT_NO_LISTEN;
    info.iface = NULL;
    info.protocols = protocols;
    info.extensions = exts;
    info.options = LWS_SERVER_OPTION_VALIDATE_UTF8 | LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    info.gid = -1;
    info.uid = -1;
    info.server_string = NULL;
    info.user = context;

    lws_context = lws_create_context(&info);
    if(lws_context == NULL) {
        ld_err(context, "lws context init failed while trying to connect to the gateway");
        return -1;
    }
    context->lws_context = lws_context;

    i->context = lws_context;

    char *gateway_url;
    gateway_url = malloc(1000);
    sprintf(gateway_url, "/?v=%d&encoding=json", LD_WS_API_VERSION);
    i->address = context->gateway_bot_url + 6; //omit "wss://" part

    i->port = 443;
    i->ssl_connection = 1;
    i->path = gateway_url;

    char *ads_port;
    ads_port = malloc((strlen(i->address) + 10) * sizeof(char));
    sprintf(ads_port, "%s:%u", i->address, 443&65535);
    i->host = ads_port;
    i->origin = ads_port;

    i->protocol = protocols[0].name;

    ld_debug(context, "connecting to gateway");
    struct lws *wsi;
    wsi = lws_client_connect_via_info(i);
    if(wsi == NULL) {
        ld_err(context, "failed to connect to gateway (%s)", i->address);
        return 1;
    }
    context->lws_wsi = wsi;
    free(ads_port);
    return 0;
}

int ld_gateway_resume(struct ld_context *context) {
    return 0;
}

int ld_lws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                    void *user, void *in, size_t len) {

    struct ld_context *context;
    context = lws_context_user(lws_get_context(wsi)); //retrieve ld_context pointer
    int i;
    char *payload = (char *) user;

    switch(reason) {
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            ld_err(context, "lws: error connecting to gateway: %.*s(%d)", in, len, len);
            context->gateway_state = LD_GATEWAY_DISCONNECTED;
            return -1;
            break;
        case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
            ld_info(context, "lws: recieved handshake from Discord gateway");
            return 0;
            break;
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            ld_info(context, "established websocket connection to gateway");
            break;

        case LWS_CALLBACK_WSI_DESTROY:
            context->gateway_state = LD_GATEWAY_DISCONNECTED;
            return -1;
            break;
        case LWS_CALLBACK_GET_THREAD_ID:
            return 0;
            break;
        case LWS_CALLBACK_ESTABLISHED:
            ld_info(context, "lws: established websocket connection to gateway");
            break;
        case LWS_CALLBACK_CLOSED:
            ld_notice(context, "lws: websocket connection to gateway closed");
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE: {
            char *tmp;
            tmp = malloc(len + 1);
            strncpy(tmp, in, len);
            ld_debug(context, "lws: received data from gateway: \n%s", tmp);
            free(tmp);
            }
            ld_gateway_payload_parser(context, in, len); //take the buffer and interpret it
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
            ld_debug(context, "lws: recieved pong from gateway");
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            ld_debug(context, "lws: client writable callback");
            if(context->gateway_queue == NULL) {
                ld_notice(context, "nothing in queue to send");
                break;
            }
            i = sprintf(payload + LWS_PRE, "%s", (char *) context->gateway_queue);
            if(i <= 0) {
                ld_err(context, "couldn't write payload to buffer");
                return -1;
            }
            lwsl_notice("TX: %s\n", payload + LWS_PRE);
            i = lws_write(wsi, (unsigned char *) (payload + LWS_PRE), strlen(context->gateway_queue), LWS_WRITE_TEXT);
            if(i < 0) {
                lwsl_err("ERROR %d writing to socket, hanging up\n", i);
                return -1;
            }
            if(i < strlen(context->gateway_queue)) {
                lwsl_err("Partial write\n");
                return -1;
            }
            free(context->gateway_queue);
            context->gateway_queue = NULL;
            break;
        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
            ld_info(context, "lws: gateway initiated close of websocket: close code: %u\nCONTEXT:\n%s", (unsigned int) (( unsigned char *)in)[0] << 8 | (( unsigned char *)in)[1], in+2);
            context->close_code = (unsigned int) (( unsigned char *)in)[0] << 8 | (( unsigned char *)in)[1];
            break;
        default:
            ld_debug(context, "lws: received lws callback reason %d", reason);
            break;
    }
    return 0;
}

/*
 *
 */
enum ld_gateway_payloadtype ld_gateway_payload_objectparser(const char *key) {
    //compare key to d,t,s,op and return appropriate enum
    int cmp;
    cmp = strcmp(key, "op");
    if(cmp == 0)
        return LD_GATEWAY_OP;
    cmp = strcmp(key, "d");
    if(cmp == 0)
        return LD_GATEWAY_D;
    cmp = strcmp(key, "t");
    if(cmp == 0)
        return LD_GATEWAY_T;
    cmp = strcmp(key, "s");
    if(cmp == 0)
        return LD_GATEWAY_S;

    return LD_GATEWAY_UNKNOWN;
}

json_t *_ld_generate_identify(struct ld_context *context) {
    json_t *ident;
    json_error_t error;
    //token:string, token
    ident = json_pack_ex(&error, 0, "{"
                    "ss" //token
                    "si" //large_threshold
                    "sb" //compress
                    "s[ii]" //shard
                    "s{ss ss ss}" //properties {$os, $browser, $device}
                    "s{" //presence
                      "s{ss si}" //game {name, type, url?}
                      "ss" //status
                      "so?" //since
                      "sb}}", //afk
    "token", context->bot_token,
    "large_threshold", 250,
    "compress", 0, //todo: implement some kind of compression
    "shard", 0, context->shards,
    "properties",
              "$os", "Linux",
              "$browser", "libdiscord",
              "$device", "libdiscord",
    "presence",
        "game",
            "name", "for alienz",
            "type", 3,
            //NULL, NULL,
        "status", "online",
        "since", NULL,
        "afk", 0
    );
    if(ident == NULL) {
        ld_err(context, "error generating IDENTIFY payload: %s\n"
                "source: \n%s\nin line %d column %d and position %d",
               error.text, error.source, error.line, error.column, error.position);

    }

    return ident;
}

int ld_gateway_payload_parser(struct ld_context *context, char *in, size_t len) {
    //parse as JSON
    json_t *payload, *value, *tmp;
    json_t *d, *t, *s, *op;
    json_error_t error;
    const char *key;
    payload = json_loadb(in, len, 0, &error);
    if(payload == NULL) {
        ld_warn(context, "couldn't parse payload from gateway");
        return 1;
    }
    enum ld_gateway_opcode opcode = LD_GATEWAY_OPCODE_UNKNOWN;

    json_object_foreach(payload, key, value) {
        /*
         * gateway payloads can have up to four fields in the highest level:
         *  op, t, s, d
         * not all fields are guaranteed to exist
         * 'd' (data) can have many objects inside it, depending on the opcode. Variable JSON type
         * 'op' (opcode) should always be specified, always integer type
         * 's' (sequence number) only comes with opcode 0, always integer type
         * 't' (type) event name, only comes with opcode 0, always string type
         */
        switch (ld_gateway_payload_objectparser(key)) {
            case LD_GATEWAY_OP:
                opcode = (enum ld_gateway_opcode) json_integer_value(value);
                ld_debug(context, "received opcode %d", opcode);
                break;
            case LD_GATEWAY_D:
                d = value;
                ld_debug(context, "got data field in payload");
                break;
            case LD_GATEWAY_T:
//                t = value;
                break;
            case LD_GATEWAY_S:
                context->last_seq = (int) json_integer_value(value);
                break;
            case LD_GATEWAY_UNKNOWN:
                break;
        }
    }

    //if it's a HELLO payload, save the details into the context
    unsigned int hbi = 41250;
    if(opcode == LD_GATEWAY_OPCODE_HELLO) {
        //save heartbeat interval
        if(d == NULL) {
            ld_warn(context, "couldn't get d field in hello payload");
        }
        tmp = json_object_get(d, "heartbeat_interval");
        if(tmp != NULL) {
            if(json_integer_value(tmp) != 0) {
                hbi = (unsigned int) json_integer_value(tmp);
            } else {
                ld_warn(context, "unexpected type for heartbeat interval in "
                        "hello payload (not integer)");
            }
        } else {
            ld_warn(context, "couldn't find heartbeat interval in hello payload");
        }
        ld_debug(context, "heartbeat interval is %d", hbi);
        context->heartbeat_interval = hbi;

        //prepare and send a IDENTIFY payload
        op = json_integer(LD_GATEWAY_OPCODE_IDENTIFY);
        t = NULL;
        s = NULL;
        d = _ld_generate_identify(context);
        payload = ld_json_create_payload(NULL, op, d, t, s);
        context->gateway_queue = strdup(json_dumps(payload, 0));
        json_decref(payload);
        ld_debug(context, "prepared JSON identify payload: \n%s", (char *)context->gateway_queue);

        lws_callback_on_writable(context->lws_wsi);
    }


    return 0;
}

json_t *ld_json_create_payload(struct ld_context *context, json_t *op, json_t *d, json_t *t, json_t *s) {
    json_t *payload;
    payload = json_object();
    int ret;
    ret = json_object_set_new(payload, "op", op);
    if(ret != 0) {
        ld_warn(context, "couldn't set opcode in new payload");
        return NULL;
    }
    ret = json_object_set_new(payload, "d", d);
    if(ret != 0) {
        ld_warn(context, "couldn't set data in new payload");
        return NULL;
    }
    if(t != NULL) {
        ret = json_object_set_new(payload, "t", t);
        if(ret != 0) {
            ld_warn(context, "couldn't set type in new payload");
            return NULL;
        }
    }
    if(s != NULL) {
        ret = json_object_set_new(payload, "s", s);
        if(ret != 0) {
            ld_warn(context, "couldn't set sequence number in new payload");
            return NULL;
        }
    }


    return payload;
}