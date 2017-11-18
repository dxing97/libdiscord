#include <sched.h>
#include <stdlib.h>
#include "libdiscord.h"

struct ld_context *ld_create_context_via_info(struct ld_context_info *info) {
    //assuming the values passed in are good
    struct ld_context *context;
    context = malloc(sizeof(struct ld_context));

    context->log_level = info->log_level;

    context->gateway_connected = 0;
    context->gateway_disconnected = 0;
    context->gateway_unconnected = 1;

    if(info->bot_token == NULL) {
        return NULL;
    }
    return context;
}

void ld_destroy_context(struct ld_context *context) {
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
void ld_debug(struct ld_context *context, const char *message, ...) {
    if((ld_log_debug & context->log_level) != 0) {
        va_list myargs;
        va_start(myargs, message);
        _ld_log(ld_log_debug, context->log_level, message, myargs);
        va_end(myargs);
    }
}

int ld_connect(struct ld_context *context) {
    //check to see if we can even connect to Discord's servers
        //GET gateway
    //check the bot token's validity
        //GET /gateway/bot
    //connect to the websocket
    if((context->gateway_unconnected == 0) || (context->gateway_disconnected == 0)) {
        //we're not connected, so we should connect
        return 0;
    }
    //we're already connected...
    return 1;
}

int ld_connection_state(struct ld_context *context) {
    //use defines/const for this part
    if(context->gateway_connected) {
        return 1;
    }
    return 0;
}
