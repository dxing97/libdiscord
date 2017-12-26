#ifndef LIBDISCORD_0_3_LIBRARY_H
#define LIBDISCORD_0_3_LIBRARY_H

#include <curl/curl.h>
#include "libdiscord_config.h"
#include "log.h"

/*
 * LD_WEBSOCKET_RECEIVE: We've received data from the gateway
 * LD_WEBSOCKET_SENDABLE: We can now send data to the gateway
 * LD_WEBSOCKET_CONNECTING: We're connecting to the gateway (why do we need this one?)
 */
enum ld_callback_reason {
    LD_WEBSOCKET_RECEIVE = 0,
    LD_WEBSOCKET_SENDABLE = 1,
    LD_WEBSOCKET_CONNECTING = 2
};

enum gateway_state {
    LD_GATEWAY_UNCONNECTED = 0,
    LD_GATEWAY_DISCONNECTED = 1,
    LD_GATEWAY_CONNECTING = 2,
    LD_GATEWAY_CONNECTED = 3
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
    char *gateway_url;
    char *gateway_bot_url;
    enum gateway_state gateway_state;
    int shards;
    CURLM *curl_multi_handle;
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
 * returns enum corresponding to the gateway connection state
 */
int ld_gateway_connection_state(struct ld_context *context);

/*
 *
 */
int ld_gateway_connected(struct ld_context *context);

/*
 * connect to discord
 * HTTP authorization initialization
 *  check the bot token's validity here
 * websocket/gateway connection and initialization
 * returns 0 on success
 * returns 1 on connection error
 * returns 2 for a curl error
 * returns 3 for a jansson error (JSON parsing error: didn't get what we were expecting)
 */
int ld_connect(struct ld_context *context);

/*
 * services pending HTTP and websocket requests.
 */
int ld_service(struct ld_context *context);

/*
 * starts a fresh gateway connection
 * will:
 *  start a fresh websocket connection
 *  parse HELLO payload
 *  create & send IDENTIFY payload
 *  start heartbeat mechanism
 * returns 0 on success
 */
int ld_gateway_connect(struct ld_context *context);

/*
 * reconnects to the gateway (resume payload)
 */
int ld_gateway_resume(struct ld_context *context);

/*
 * callback used for lws
 */
int ld_lws_callback(struct lws *wsi, enum lws_callback_reasons reason,
                    void *user, void *in, size_t len);


#endif