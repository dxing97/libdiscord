#include <stdlib.h>
#include <jansson.h>
//#include "libwebsockets/lib/libwebsockets.h"

#include "libdiscord.h"


/*
 * libwebsockets protocols struct, parameters set here are used to set the behavior of 
 * the websocket interface, and while negotiating a websocket connection to 
 * Discord's gateway. Of particular importance is the callback function.
 */
static struct lws_protocols protocols[] = {
        {
                "DiscordBot",
                ld_lws_callback,
                8192,
                8192 //rx buffer size
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
        ld_error("bot token is null");
        return NULL;
    }
    context->bot_token = malloc(strlen(info->bot_token) + 1);
    context->bot_token = strcpy(context->bot_token, info->bot_token);

    lws_set_log_level(31, NULL);

    context->gateway_ring = lws_ring_create(
            sizeof(struct ld_gateway_payload),
            info->gateway_ringbuffer_size,
            NULL);
    if(context->gateway_ring == NULL) {
        ld_error("couldn't init gateway ringbuffer");
        return NULL;
    }

    context->presence = malloc(sizeof(struct _ld_json_presence));
    if(context->presence == NULL) {
        ld_error("couldn't allocate presence");
        return NULL;
    }
    context->presence->game = strdup(info->init_presence->game);
    context->presence->game_type = info->init_presence->game_type;
    context->presence->status_type = LD_PRESENCE_ONLINE;

    context->gateway_bot_limit = 1;
    context->gateway_bot_remaining = 1;
    context->gateway_bot_reset = lws_now_secs();

    context->gi_count = 0;
    return context;
}

void ld_destroy_context(struct ld_context *context) {
    free(context->presence->game);
    free(context->gateway_session_id);
    curl_multi_cleanup(context->curl_multi_handle);
    curl_global_cleanup();
    lws_context_destroy(context->lws_context);
    lws_ring_destroy(context->gateway_ring);
    free(context);
}
/*
void _ld_err(struct ld_context *context, const char *message, ...) {
    if((LD_LOG_ERROR & context->log_level) != 0) {
        va_list myargs;
        va_start(myargs, message);
        _ld_log(LD_LOG_ERROR, context->log_level, message, myargs);
        va_end(myargs);
    }
}

void _ld_warn(struct ld_context *context, const char *message, ...) {
    if((LD_LOG_WARNING & context->log_level) != 0) {
        va_list myargs;
        va_start(myargs, message);
        _ld_log(LD_LOG_WARNING, context->log_level, message, myargs);
        va_end(myargs);
    }
}
void _ld_info(struct ld_context *context, const char *message, ...) {
    if((LD_LOG_INFO & context->log_level) != 0) {
        va_list myargs;
        va_start(myargs, message);
        _ld_log(LD_LOG_INFO, context->log_level, message, myargs);
        va_end(myargs);
    }
}
void _ld_note(struct ld_context *context, const char *message, ...) {
    if((LD_LOG_NOTICE & context->log_level) != 0) {
        va_list myargs;
        va_start(myargs, message);
        _ld_log(LD_LOG_NOTICE, context->log_level, message, myargs);
        va_end(myargs);
    }
}
void _ld_dbug(struct ld_context *context, const char *message, ...) {\
    if((LD_LOG_DEBUG & context->log_level) != 0) {\
        va_list myargs;
        va_start(myargs, message);
        _ld_log(LD_LOG_DEBUG, context->log_level, message, myargs);
        va_end(myargs);
    }
}
*/

struct _ld_buffer {
    char *string;
    size_t size;
    struct ld_context *context;
};

/*
 * curl callback function used to read data returned from HTTP request
 */
size_t _ld_curl_response_string(void *contents, size_t size, size_t nmemb, void *userptr){
    size_t recieved_size = size * nmemb;
    struct _ld_buffer *buffer = (struct _ld_buffer *) userptr;

    buffer->string = realloc(buffer->string, buffer->size + recieved_size + 1);
    if(buffer->string == NULL) {
        ld_error("realloc: couldn't allocate memory for curl response string in ld_connect!");
    }

    memcpy(&(buffer->string[buffer->size]), contents, recieved_size);
    buffer->size += recieved_size;
    buffer->string[buffer->size] = '\0';

    return recieved_size;
}

/*
 * internal GET /gateway function
 * returns 0 on success
 * returns 1 on ulfius error
 * returns 2 on curl error
 * returns 3 on jansson error
 */
int _ld_get_gateway(struct ld_context *context) {
    /*
     * check to see if we can even connect to Discord's servers
     * examine /gateway and see if we get a valid response
     */
    int ret;
    struct ld_rest_request *request = ld_rest_init_request();
    struct ld_rest_response *response = ld_rest_init_response();

    request = ld_get_gateway(request, context);

    ret = ld_rest_send_blocking_request(request, response);

    if(ret != 0) {
        ld_error("ulfius: couldn't send request to /gateway (%d)", ret);
        return 1;
    }

    ld_debug("get gateway response: %s", response->body);

    //use jansson to extract the JSON data
    json_t *object, *tmp;
    json_error_t error;

    object = json_loads(response->body, 0, &error);
    if(object == NULL) {
        ld_error("jansson: couldn't decode string returned "
                "from /gateway in ld_connext: %s", response->body);
        return 3;
    }

    tmp = json_object_get(object, "url");
    if(tmp == NULL) {
        ld_error("jansson: couldn't find key \"url\" in JSON object from /gateway");
        return 3;
    }

    if(json_string_value(tmp) == NULL) {
        ld_error("jansson: didn't receive string object from "
                "JSON payload received from gateway");
        return 3;
    }

    context->gateway_url = malloc(strlen(json_string_value(tmp)) + 1);
    context->gateway_url = strcpy(context->gateway_url, json_string_value(tmp));

    ld_rest_free_request(request);
    ld_rest_free_response(response);
    free(tmp);
    free(object);
    return 0;
}

/*
 * curl callback function
 * prints headers
 */
size_t ld_curl_header_parser(char *buffer, size_t size, size_t nitems, void *userdata) {
    char *tmp;

    if(size*nitems == 2) {
        return 2;
    }

    tmp = strndup(buffer, size*nitems - 1);
    ld_debug("headers(%d): %s", (int) nitems * size, tmp);
    free(tmp);
    return size*nitems;
}

/*
 * internal /gateway/bot function
 * returns 0 on success
 * returns 2 on curl error
 * returns 3 on jansson error
 */
int _ld_get_gateway_bot(struct ld_context *context){
    /*
     * check ratelimits first
     * we got a valid response from the REST API, which should mean
     *  Discord is connectable at basic level
     *  Now we should check the bot token validity using /gateway/bot
     */
    int ret;
    struct ld_rest_request *request = ld_rest_init_request();
    struct ld_rest_response *response = ld_rest_init_response();
    request = ld_get_gateway_bot(request, context);
    ret = ld_rest_send_blocking_request(request, response);
    if(ret != 0) {
        ld_error("couldn't send request to /gateway/bot");
    }
    ld_debug("get gateway bot response: %s", response->body);

    json_t *object, *tmp;
    json_error_t error;

    object = json_loads(response->body, 0, &error);
    if(object == NULL) {
        ld_error("jansson: couldn't decode string returned "
                "from /gateway/bot in ld_connect: %s", response->body);
        return 3;
    }

    tmp = json_object_get(object, "url");
    if(tmp == NULL) {
        ld_error("jansson: couldn't find key \"url\" in JSON object from /gateway/bot."
                " is the bot token valid? are we being ratelimited?");
        return 3;
    }

    if(json_string_value(tmp) == NULL) {
        ld_error("jansson: didn't receive string object in \"url\" from "
                "JSON payload received from /gateway/bot");
        return 3;
    }

    context->gateway_bot_url = malloc(strlen(json_string_value(tmp)) + 1);
    context->gateway_bot_url = strcpy(context->gateway_url, json_string_value(tmp));

    tmp = json_object_get(object, "shards");
    if(tmp == NULL) {
        ld_error("jansson: couldn't find key \"shards\" in JSON object from /gateway/bot."
                "is the bot token valid?");
        return 3;
    }

    if(json_integer_value(tmp) == 0) {
        ld_error("jansson: didn't receive integer object in \"shards\" from "
                "JSON payload received from /gateway/bot");
        return 3;
    }

    context->shards = (int) json_integer_value(tmp);
    ld_info("shards: %d", context->shards);
    return 0;
}

struct ld_gi *_ld_init_gi(struct ld_context *context) {
    //initialize the gateway interface
    context->gi = malloc(sizeof(struct ld_gi));
    if(context->gi == NULL) {
        ld_error("couldn't allocate memory for gateway interface struct");
        return NULL;
    }

    struct ld_gi *gi = context->gi[0];

    gi->parent_context = context;
    gi->state = LD_GATEWAY_UNCONNECTED;
    gi->shardnum = -1;
    gi->lws_wsi = NULL;
    gi->hb_interval = 41500; //will be overwritten later
    gi->last_seq = 0;
    gi->hb_count = 0;
    gi->tx_ringbuffer = NULL;
    gi->close_code = 0;
    gi->session_valid = 0;

    return gi;
}

int ld_connect(struct ld_context *context) {
    int ret;
    /*
     * initiates a connection to Discord, mainly though the gateway.
     * First it GETs the gateway URL from /gateway, mainly done to make sure we even have access to the internet.
     * Then it uses the bot token to GET shard information from /gateway/bot and to make sure the bot token
     * is valid.
     * Then it checks the context state and determines what should be done next.
     * If we're unconnected, it'll call ld_connect
     * If we're disconnected, it'll call gateway_resume
     */
    if(context->gateway_url == NULL) {
        ret = _ld_get_gateway(context);
        if(ret != 0) {
            ld_error("couldn't get gateway URL from /gateway");
            return ret;
        }
    }

    //todo: add init for context.gi here
    //init the gateway interface

    switch(context->gateway_state) {
        case LD_GATEWAY_UNCONNECTED:
            ret = _ld_get_gateway_bot(context);
            if(ret != 0) {
                ld_error("couldn't get gateway URL from /gateway/bot");
                return ret;
            }

            context->gateway_state = LD_GATEWAY_CONNECTING;

            ret = ld_gateway_connect(context);

            if(ret != 0){
                return 1;
            }

            break;
        case LD_GATEWAY_CONNECTING:
            //??? do nothing
            ld_notice("ld_connect called while connecting to gateway!");
            break;
        case LD_GATEWAY_CONNECTED:
            //this context already has an established connection to the gateway
            break;
        default:
            //???
            break;
    }

    return 0;
}

int ld_service(struct ld_context *context, int timeout) {
    int ret = 0;

    if(context->gateway_state == LD_GATEWAY_UNCONNECTED) {
        //we are unconnected, let's check if there was a close code
        //we are trying to service a closed connection, so we should try opening that connection
        ld_connect(context);
        return 0;
    }

    //check heartbeat timer
    if((lws_now_secs() - context->last_hb) > (context->heartbeat_interval/1000)) {
        //put heartbeat payload in gateway_queue
        context->hb_count++;
        if(context->hb_count > 1){
            //didn't receive HB_ACK
            ld_warning("ld_service: didn't recieve a HB_ACK");
            return LD_HB_ACKMISS;
        }
        ret = ld_gateway_queue_heartbeat(context);
        if(ret != 0) {
            ld_warning("couldn't put heartbeat into gateway tx ringbuffer");
            context->hb_count--;
            return 1;
        }

        context->last_hb = lws_now_secs();
    }

    if(context->gateway_state == LD_GATEWAY_UNCONNECTED) {
        //something happened and we were disconnected from the gateway, or we were never connected in the first place
        ld_warning("we were disconnected from the gateway!");
    }

    //call writable if there's things that need to be sent
    if(lws_ring_get_count_waiting_elements(context->gateway_ring, NULL) != 0)
        lws_callback_on_writable(context->lws_wsi);

    lws_service(context->lws_context, timeout);
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
        ld_error("lws context init failed while trying to connect to the gateway");
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

    ld_debug("connecting to gateway");
    struct lws *wsi;
    wsi = lws_client_connect_via_info(i);
    if(wsi == NULL) {
        ld_error("failed to connect to gateway (%s)", i->address);
        return 1;
    }
    context->lws_wsi = wsi;
    free(ads_port);
    return 0;
}

int ld_gateway_resume(struct ld_context *context) {
    //doesn't do anything yet
    return 0;
}

int ld_lws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                    void *user, void *in, size_t len) {

    struct ld_context *context;
    context = lws_context_user(lws_get_context(wsi)); //retrieve ld_context pointer
    int i;
    char *payload = (char *) user;
    struct ld_gateway_payload *gateway_payload;
    char *close_message = NULL;

    switch(reason) {
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            ld_error("lws: error connecting to gateway: %.*s(%d)", in, len, len);
            context->user_callback(context, LD_CALLBACK_WS_CONNECTION_ERROR, NULL, 0);
            return 0;
        case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
            ld_info("lws: received handshake from Discord gateway");
            return 0;
            break;
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            ld_info("established websocket connection to gateway");
            i = context->user_callback(context, LD_CALLBACK_WS_ESTABLISHED, NULL, 0);
            context->gateway_state = LD_GATEWAY_CONNECTED;
            return i;

        case LWS_CALLBACK_GET_THREAD_ID:
            return 0;
            break;
        case LWS_CALLBACK_CLOSED:
            ld_notice("lws: websocket connection to gateway closed");
            break;
        case LWS_CALLBACK_CLIENT_RECEIVE:
            //check to see if we've received a new fragment
            ld_debug("first?=%d, last=%d", lws_is_first_fragment(wsi), lws_is_final_fragment(wsi));
            if(context->gateway_rx_buffer == NULL && lws_is_final_fragment(wsi)) {//first fragment is also the last fragment
                //we're
                ld_notice("first gateway fragment is also last fragment");
                i = ld_gateway_payload_parser(context, in, len); //take the buffer and interpret it

                ld_notice("single RX: %s", (char *) in);
                context->gateway_rx_buffer = NULL;
                context->gateway_rx_buffer_len = 0;

                return i;
            }

            if (lws_is_first_fragment(wsi)) { //first fragment
                //new fragment
                context->gateway_rx_buffer = malloc(len + 1);
                strncpy(context->gateway_rx_buffer, in, len);
                context->gateway_rx_buffer_len = len;

                return 0;
            }
            if(lws_is_final_fragment(wsi)) { //last fragment of multi-fragment payload
                /*
                 * append fragment to previous fragment
                 */
                context->gateway_rx_buffer = realloc(context->gateway_rx_buffer,
                                                     context->gateway_rx_buffer_len + len + 2);
                strncpy(context->gateway_rx_buffer + context->gateway_rx_buffer_len, in, len);
                context->gateway_rx_buffer_len += len;
                context->gateway_rx_buffer[context->gateway_rx_buffer_len] = '\0';
                if(context->gateway_rx_buffer_len > 8000) {
                    ld_notice("multi RX: %.*s...", 2000, context->gateway_rx_buffer);
                } else {
                    ld_notice("multi RX: %s", context->gateway_rx_buffer);
                }

                i = ld_gateway_payload_parser(context, context->gateway_rx_buffer, context->gateway_rx_buffer_len);

                ld_debug("payload parser code: %d", i);
                free(context->gateway_rx_buffer);
                context->gateway_rx_buffer = NULL;
                context->gateway_rx_buffer_len = 0;

                return i;
            }
            //not first or last fragment
            context->gateway_rx_buffer = realloc(context->gateway_rx_buffer,
                                                 context->gateway_rx_buffer_len + len + 1);
            strncpy(context->gateway_rx_buffer + context->gateway_rx_buffer_len, in, len);
            context->gateway_rx_buffer_len += len;

//            _ld_note(context, "multi RX: %.*s", context->gateway_rx_buffer, context->gateway_rx_buffer_len);

            return 0;
        case LWS_CALLBACK_CLIENT_RECEIVE_PONG:
            ld_debug("lws: recieved pong from gateway");
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            ld_debug("lws: client writable callback");
            gateway_payload = malloc(sizeof(struct ld_gateway_payload));

            if(lws_ring_get_count_waiting_elements(context->gateway_ring, NULL) == 0) {
                ld_debug("nothing in queue to send");
                break;
            }

            i = (int) lws_ring_consume(context->gateway_ring, NULL, gateway_payload, 1);
            if(i != 1) {
                ld_warning("couldn't consume payload from ringbuffer");
                break;
            }

            i = sprintf(payload + LWS_PRE, "%s", (char *) gateway_payload->payload);
            if(i <= 0) {
                ld_error("couldn't write payload to buffer");
                return -1;
            }
            lwsl_notice("TX: %s\n", payload + LWS_PRE);
            i = lws_write(wsi, (unsigned char *) (payload + LWS_PRE), strlen(gateway_payload->payload), LWS_WRITE_TEXT);
            if(i < 0) {
                lwsl_err("ERROR %d writing to socket, hanging up\n", i);
                return -1;
            }
            if(i < strlen(gateway_payload->payload)) {
                lwsl_err("Partial write\n");
                return -1;
            }
            free(gateway_payload->payload);
            free(gateway_payload);
//            context->gateway_queue = NULL;
            break;
        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
            ld_info("lws: gateway initiated close of websocket: "
                             "close code: %u\nCONTEXT (%d):\n%.*s",
                     (unsigned int) ((unsigned char *) in)[0] << 8 | ((unsigned char *) in)[1], len, len, in + 2);
            context->close_code = (unsigned int) (( unsigned char *)in)[0] << 8 | (( unsigned char *)in)[1];
            context->gateway_state = LD_GATEWAY_UNCONNECTED;
            if(len != 0) {
                close_message = strndup(in + 1, len);
            }

            i = context->user_callback(context, LD_CALLBACK_WS_PEER_CLOSE, close_message, context->close_code);

            if(len != 0) {
                free(close_message);
            }

            // set context.gi.resume here
            if((context->close_code == 1000) || (context->close_code == 4006)) {
                //opcode 9 must have been received by now (websocket is already closing)
                context->gi[0]->session_valid = 0; //todo: WHICH GI DOES THIS WSI CORRESPOND TO???
            }
            return 0;
        case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
        case LWS_CALLBACK_LOCK_POLL:
        case LWS_CALLBACK_UNLOCK_POLL:
            break;
        default:
            ld_debug("lws: received lws callback reason %d", reason);
            break;
    }
    return 0;
}

/*
 * returns payload type enum
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

const char *ld_presence_status_to_str(enum ld_presence_status_type type){
    switch(type) {
        case LD_PRESENCE_IDLE:
            return "idle";
        case LD_PRESENCE_DND:
            return "dnd";
        case LD_PRESENCE_ONLINE:
            return "online";
        case LD_PRESENCE_OFFLINE:
            return "offline";
    }
    return NULL;
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
                      "sb}" //afk
    "}"
            ,
    "token", context->bot_token,
    "large_threshold", 250,
    "compress", 0,
    "shard", 0, context->shards,
    "properties",
              "$os", "Linux",
              "$browser", "libdiscord",
              "$device", "libdiscord"
            ,
    "presence",
        "game",
            "name", context->presence->game,
            "type", context->presence->game_type,
            //NULL, NULL,
        "status", ld_presence_status_to_str(context->presence->status_type),
        "since", NULL,
        "afk", 0
    );
    if(ident == NULL) {
        ld_error("error generating IDENTIFY payload: %s\n"
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
        ld_debug("couldn't parse payload from gateway: %s", error.text);
        return 1;
    }
    d = NULL;
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
                ld_debug("received opcode %d", opcode);
                break;
            case LD_GATEWAY_D:
                d = value;
                ld_debug("got data field in payload");
                break;
            case LD_GATEWAY_T:
                t = value;
                break;
            case LD_GATEWAY_S:
                context->last_seq = (int) json_integer_value(value);
                break;
            case LD_GATEWAY_UNKNOWN:
                ld_warning("got unknown key in payload");
                return 1;
        }
    }

    //if it's a HELLO payload, save the details into the context.
    //This must be handled first so we'll do it here
    unsigned int hbi = 41250;
    switch (opcode) {
        case LD_GATEWAY_OPCODE_DISPATCH:
            //check t for dispatch type
            return ld_gateway_dispatch_parser(context, t, d);
        case LD_GATEWAY_OPCODE_HEARTBEAT:
            //can be sent by the gateway every now and then
            break;
        case LD_GATEWAY_OPCODE_IDENTIFY:break;
        case LD_GATEWAY_OPCODE_PRESENCE:break;
        case LD_GATEWAY_OPCODE_VOICE_STATE:break;
        case LD_GATEWAY_OPCODE_VOICE_PING:break;
        case LD_GATEWAY_OPCODE_RESUME:break;
        case LD_GATEWAY_OPCODE_RECONNECT:break;
        case LD_GATEWAY_OPCODE_REQUEST_MEMBERS:break;
        case LD_GATEWAY_OPCODE_INVALIDATE_SESSION:
            ld_info("gateway session invalidated");
            context->gi[0]->session_valid = 0; //todo: WHICH WSI DOES THIS GI CORRESPOND TO??
            break;
        case LD_GATEWAY_OPCODE_HELLO:
            //save heartbeat interval

            if(d == NULL) {
                ld_warning("couldn't get d field in hello payload");
            }
            tmp = json_object_get(d, "heartbeat_interval");
            if(tmp != NULL) {
                if(json_integer_value(tmp) != 0) {
                    hbi = (unsigned int) json_integer_value(tmp);
                } else {
                    ld_warning("unexpected type for heartbeat interval in "
                            "hello payload (not integer)");
                }
            } else {
                ld_warning("couldn't find heartbeat interval in hello payload");
            }
            ld_debug("heartbeat interval is %d", hbi);
            context->heartbeat_interval = hbi;

            //prepare IDENTIFY payload
            op = json_integer(LD_GATEWAY_OPCODE_IDENTIFY);
            t = NULL;
            s = NULL;
            d = _ld_generate_identify(context);

            if(d == NULL) {
                ld_error("couldn't generate identify payload, closing gateway connection");
                return 1;
            }

            payload = ld_json_create_payload(op, d, t, s);

            struct ld_gateway_payload *toinsert;
            toinsert = malloc(sizeof(struct ld_gateway_payload));
            toinsert->payload = strdup(json_dumps(payload, 0));
            toinsert->len = strlen(toinsert->payload);
            lws_ring_insert(context->gateway_ring, toinsert, 1);

            json_decref(payload);
            ld_debug("prepared JSON identify payload: \n%s", (char *) toinsert->payload);

//            lws_callback_on_writable(context->lws_wsi); //send identify payload

            //set heartbeat timer here
            context->last_hb = lws_now_secs(); //seconds since 1970-1-1
            break;
        case LD_GATEWAY_OPCODE_HEARTBEAT_ACK:
            context->hb_count--;
            break;
        case LD_GATEWAY_OPCODE_GUILD_SYNC:
            //only seen on user accounts
            break;
        case LD_GATEWAY_OPCODE_UNKNOWN:
        default:
            ld_warning("received payload with unknown opcode");
            return 1;
    }


    return 0;
}


int ld_dispatch_ready(struct ld_context *context, json_t *data) {
    //save session_id
    const char *key;
    json_t *value;
    json_object_foreach(data, key, value) {
        if(strcmp(key, "session_id") == 0) {
            ld_debug("gateway session_id: %s", json_string_value(value));
            context->gateway_session_id = strdup(json_string_value(value));
            return 0;
        }
    }
    return 1;
}

int ld_gateway_dispatch_parser(struct ld_context *context, json_t *type, json_t *data) {
    //check type
    ld_debug("parsing dispatch type");
    int i, ret;
    const char *typestr;
    typestr = json_string_value(type);
    if(typestr == NULL) {
        ld_warning("jansson: couldn't identify gateway dispatch type");
        return 1;
    }

    //maybe have this be initialized at context init time or make it global
    struct ld_dispatch dispatch_dict[] = {
            {"READY", LD_CALLBACK_READY, &ld_dispatch_ready},
            {"CHANNEL_CREATE", LD_CALLBACK_CHANNEL_CREATE},
            {"CHANNEL_UPDATE", LD_CALLBACK_CHANNEL_UPDATE},
            {"CHANNEL_DELETE", LD_CALLBACK_CHANNEL_DELETE},
            {"CHANNEL_PINS_UPDATE", LD_CALLBACK_CHANNEL_PINS_UPDATE},
            {"GUILD_CREATE", LD_CALLBACK_GUILD_CREATE},
            {"GUILD_UPDATE", LD_CALLBACK_GUILD_UPDATE},
            {"GUILD_DELETE", LD_CALLBACK_GUILD_DELETE},
            {"GUILD_BAN_ADD", LD_CALLBACK_GUILD_BAN_ADD},
            {"GUILD_BAN_REMOVE", LD_CALLBACK_GUILD_BAN_REMOVE},
            {"GUILD_EMOJIS_UPDATE", LD_CALLBACK_GUILD_EMOJIS_UPDATE},
            {"GUILD_INTEGRATIONS_UPDATE", LD_CALLBACK_GUILD_INTEGRATIONS_UPDATE},
            {"GUILD_MEMBER_ADD", LD_CALLBACK_GUILD_MEMBER_ADD},
            {"GUILD_MEMBER_REMOVE", LD_CALLBACK_GUILD_MEMBER_REMOVE},
            {"GUILD_MEMBER_UPDATE", LD_CALLBACK_GUILD_MEMBER_UPDATE},
            {"GUILD_MEMBERS_CHUNK", LD_CALLBACK_GUILD_MEMBERS_CHUNK},
            {"GUILD_ROLE_CREATE", LD_CALLBACK_GUILD_ROLE_CREATE},
            {"GUILD_ROLE_UPDATE", LD_CALLBACK_GUILD_ROLE_UPDATE},
            {"GUILD_ROLE_DELETE", LD_CALLBACK_GUILD_ROLE_DELETE},
            {"MESSAGE_CREATE", LD_CALLBACK_MESSAGE_CREATE},
            {"MESSAGE_UPDATE", LD_CALLBACK_MESSAGE_UPDATE},
            {"MESSAGE_DELETE", LD_CALLBACK_MESSAGE_DELETE},
            {"MESSAGE_DELETE_BULK", LD_CALLBACK_MESSAGE_DELETE_BULK},
            {"MESSAGE_REACTION_ADD", LD_CALLBACK_MESSAGE_REACTION_ADD},
            {"MESSAGE_REACTION_REMOVE", LD_CALLBACK_MESSAGE_REACTION_REMOVE},
            {"MESSAGE_REACTION_REMOVE_ALL", LD_CALLBACK_MESSAGE_REACTION_REMOVE_ALL},
            {"PRESENCE_UPDATE", LD_CALLBACK_PRESENCE_UPDATE},
            {"TYPING_START", LD_CALLBACK_TYPING_START},
            {"USER_UPDATE", LD_CALLBACK_USER_UPDATE},
            {"VOICE_STATE_UPDATE", LD_CALLBACK_VOICE_STATE_UPDATE},
            {"VOICE_SERVER_UPDATE", LD_CALLBACK_VOICE_SERVER_UPDATE},
            {"WEBHOOKS_UPDATE", LD_CALLBACK_WEBHOOKS_UPDATE},
            {NULL, LD_CALLBACK_UNKNOWN} //null terminator
    };
    for(i = 0; dispatch_dict[i].name != NULL; i++) {
        if(strcmp(typestr, dispatch_dict[i].name) == 0) {
            ld_debug("dispatch type is %s, callback reason is %d", dispatch_dict[i].name,
                     dispatch_dict[i].cbk_reason);
            if(dispatch_dict[i].dispatch_callback != NULL) {
                ld_debug("calling dispatch libdiscord callback for ld_callback (%s)", typestr);
                ret = dispatch_dict[i].dispatch_callback(context, data);
                return ret;
            }
            i = context->user_callback(context, dispatch_dict[i].cbk_reason, data, 0);
            return i;
        }
    }
    return 0;
}

int ld_gateway_queue_heartbeat(struct ld_context *context) {
    size_t ret;
    struct ld_gateway_payload *tmp;
    json_t *hb;

    hb = ld_json_create_payload(json_integer(LD_GATEWAY_OPCODE_HEARTBEAT),
                                json_integer(context->last_seq), NULL, NULL); //create heartbeat payload

    if(lws_ring_get_count_free_elements(context->gateway_ring) == 0) {
        ld_warning("can't fit any new payloads into gateway ringbuffer");
        return 1;
    }

    tmp = malloc(sizeof(struct ld_gateway_payload));
    tmp->len = strlen(json_dumps(hb, 0)); //JSONs can't have \0 chars
    tmp->payload = strdup(json_dumps(hb, 0));

    ret = lws_ring_insert(context->gateway_ring, tmp, 1);
    if(ret != 1){
        ld_warning("couldn't fit heartbeat payload into gateway ringbuffer");
        return 1;
    }
    return 0;
}

/*
 * calls lws_context_destroy to close the ws connection
 * sets the gateway state to unconnected
 * sets reconnection context
 */
int ld_gateway_reconnect(struct ld_context *context) {
    lws_close_reason(context->lws_wsi, LWS_CLOSE_STATUS_POLICY_VIOLATION, NULL, 0);
    lws_context_destroy(context->lws_context);

    return 0;
}
