#include <stdlib.h>
//#include "libwebsockets/lib/libwebsockets.h"

#include <jansson.h>
#include "libdiscord.h"



static struct lws_protocols protocols[] = {
        {
                "DiscordBot",
                ld_lws_callback,
                sizeof(struct ld_context *),
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
                "is the bot token valid?");
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
    i->ssl_connection = 2;
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

    ld_debug(context, "recieved lws callback reason %d", reason);
    switch(reason) {
        case LWS_CALLBACK_ESTABLISHED:break;
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            ld_err(context, "lws: error connecting to gateway: %.*s", in, len);
            context->gateway_state = LD_GATEWAY_DISCONNECTED;
            return -1;
            break;
        case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH:
            ld_info(context, "lws: recieved handshake from Discord gateway");
            return 0;
            break;
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("established connection to gateway\n");
            break;
        case LWS_CALLBACK_CLOSED:break;
        case LWS_CALLBACK_CLOSED_HTTP:break;
        case LWS_CALLBACK_RECEIVE:break;
        case LWS_CALLBACK_RECEIVE_PONG:break;
        case LWS_CALLBACK_CLIENT_RECEIVE:break;
        case LWS_CALLBACK_CLIENT_RECEIVE_PONG:break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:break;
        case LWS_CALLBACK_SERVER_WRITEABLE:break;
        case LWS_CALLBACK_HTTP:break;
        case LWS_CALLBACK_HTTP_BODY:break;
        case LWS_CALLBACK_HTTP_BODY_COMPLETION:break;
        case LWS_CALLBACK_HTTP_FILE_COMPLETION:break;
        case LWS_CALLBACK_HTTP_WRITEABLE:break;
        case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:break;
        case LWS_CALLBACK_FILTER_HTTP_CONNECTION:break;
        case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:break;
        case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:break;
        case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:break;
        case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_SERVER_VERIFY_CERTS:break;
        case LWS_CALLBACK_OPENSSL_PERFORM_CLIENT_CERT_VERIFICATION:break;
        case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:break;
        case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY:break;
        case LWS_CALLBACK_CLIENT_CONFIRM_EXTENSION_SUPPORTED:break;
        case LWS_CALLBACK_PROTOCOL_INIT:break;
        case LWS_CALLBACK_PROTOCOL_DESTROY:break;
        case LWS_CALLBACK_WSI_CREATE:break;
        case LWS_CALLBACK_WSI_DESTROY:
            context->gateway_state = LD_GATEWAY_DISCONNECTED;
            return -1;
            break;
        case LWS_CALLBACK_GET_THREAD_ID:
            return 0;
            break;
        case LWS_CALLBACK_ADD_POLL_FD:break;
        case LWS_CALLBACK_DEL_POLL_FD:break;
        case LWS_CALLBACK_CHANGE_MODE_POLL_FD:break;
        case LWS_CALLBACK_LOCK_POLL:break;
        case LWS_CALLBACK_UNLOCK_POLL:break;
        case LWS_CALLBACK_OPENSSL_CONTEXT_REQUIRES_PRIVATE_KEY:break;
        case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:break;
        case LWS_CALLBACK_WS_EXT_DEFAULTS:break;
        case LWS_CALLBACK_CGI:break;
        case LWS_CALLBACK_CGI_TERMINATED:break;
        case LWS_CALLBACK_CGI_STDIN_DATA:break;
        case LWS_CALLBACK_CGI_STDIN_COMPLETED:break;
        case LWS_CALLBACK_ESTABLISHED_CLIENT_HTTP:break;
        case LWS_CALLBACK_CLOSED_CLIENT_HTTP:break;
        case LWS_CALLBACK_RECEIVE_CLIENT_HTTP:break;
        case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:break;
        case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:break;
        case LWS_CALLBACK_USER:break;

    }
    return 0;
}