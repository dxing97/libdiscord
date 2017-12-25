#include <stdlib.h>
#include <libwebsockets.h>
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
//    ld_debug(context, "servicing gateway payloads");
    return 0;
}

int ld_gateway_connect(struct ld_context *context) {
    //lws context creation info
    struct lws_context_creation_info *info;
    struct lws_context *lws_context;
    struct lws_client_connect_info *i;

    lws_set_log_level(7, lwsl_emit_syslog);

    info = malloc(sizeof(struct lws_context_creation_info));
    memset(info, 0, sizeof(struct lws_client_connect_info));
    i = calloc(0, sizeof(struct lws_client_connect_info));

    info->port = CONTEXT_PORT_NO_LISTEN;
    info->port = 443;
    info->iface = NULL; //todo: add some way of specifying which interface lws should use
    info->protocols = protocols;
    info->extensions = exts;
    info->options = 0 | LWS_SERVER_OPTION_VALIDATE_UTF8 | LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
    info->ssl_cert_filepath = NULL;
    info->ssl_private_key_filepath = NULL;
    info->gid = -1;
    info->uid = -1;

    lws_context = lws_create_context(info);
    if(lws_context == NULL) {
        ld_err(context, "lws init failed trying to connect to the gateway");
        return -1;
    }
    i->context = lws_context;
    char gateway_url[1000];
    sprintf(gateway_url, "%s/?v=%d&encoding=json", context->gateway_bot_url, LD_WS_API_VERSION);
    i->address = gateway_url;
    return 0;
}

int ld_gateway_resume(struct ld_context *context) {
    return 0;
}

int ld_lws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                    void *user, void *in, size_t len) {
    return 0;
}