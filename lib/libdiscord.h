#ifndef LIBDISCORD_0_3_LIBRARY_H
#define LIBDISCORD_0_3_LIBRARY_H

#include "libdiscord_config.h"
#include "log.h"

/*
 * LD_GATEWAY_RECIEVE: We've receieved data from the gateway
 * LD_GATEWAY_SENDABLE: We can now send data to the gateway
 * LD_GATEWAY_CONNECTING: We're connecting to the gateway (why do we need this one?)
 */
enum ld_callback_reason {
    LD_GATEWAY_RECIEVE = 0,
    LD_GATEWAY_SENDABLE = 1,
    LD_GATEWAY_CONNECTING = 2
};

/*
 * context for each bot
 * one context can have multiple gateway (websocket) connections to discord
 *  sharding, voice connections
 * gateway_connected: are we connected to the gateway?
 * gateway_disconnected: were we disconnected to the gateway?
 * gateway_unconnected: have we ever connected to the gateway?
 * user_callback: user defined callback function for event loops.
 */
struct ld_context {
    char *bot_token;
    void *user_data;
    unsigned long log_level;
    char *rest_base_url;
    int gateway_connected;
    int gateway_disconnected;
    int gateway_unconnected;
    int gateway_connecting;
    int (*user_callback)(struct ld_context *context, enum ld_callback_reason reason, const char *data, int len);

};

/*
 * info used to generate a context
 * includes:
 *  bot token
 *  user-defined pointer to anything, can be metadata about the bot (creator, version, etc.)
 *  libdiscord logging level (see ld_log_level)
 */
struct ld_context_info {
    char *bot_token;
    unsigned long log_level;
    int (*user_callback)(struct ld_context *context, enum ld_callback_reason reason, const char *data, int len);
};





/*
 * logging functions for different levels
 * context is needed to determine enabled logging levels
 */

void ld_err(struct ld_context *context, const char *message, ...);
void ld_warn(struct ld_context *context, const char *message, ...);
void ld_info(struct ld_context *context, const char *message, ...);
void ld_notice(struct ld_context *context, const char *message, ...);
void ld_debug(struct ld_context *context, const char *message, ...);

/*
 * create a context from user info
 * returns NULL if the info struct was malformed or missing things
 */
struct ld_context* ld_create_context_via_info(struct ld_context_info *info);

/*
 * destroys context
 */
void ld_destroy_context(struct ld_context *context);

/*
 * returns status of the connection to discord
 * returns 0 for not connected, nonzero for connected
 */
int ld_connection_state(struct ld_context *context);

/*
 * connect to discord
 * HTTP authorization initialization
 *  check the bot token's validity here
 * websocket/gateway connection and initialization
 * returns 0 on success
 */
int ld_connect(struct ld_context *context);

#endif