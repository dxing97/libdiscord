#ifndef LIBDISCORD_0_3_LIBRARY_H
#define LIBDISCORD_0_3_LIBRARY_H

#include <curl/curl.h>
#include <libwebsockets.h>
#include <jansson.h>
#include "libdiscord_config.h"
#include "log.h"

/*
 * LD_WEBSOCKET_RECEIVE: We've received data from the gateway
 * LD_WEBSOCKET_SENDABLE: We can now send data to the gateway
 * LD_WEBSOCKET_CONNECTING: We're connecting to the gateway (why do we need this one?)
 * LD_CALLBACK_MESSAGE_CREATE: recieved dispatch of type MESSAGE_CREATE. data is of type json_t
 */
enum ld_callback_reason {
    LD_CALLBACK_HELLO = 0, //opcode 10
    LD_CALLBACK_READY = 1, //dispatch (opcode 0)
    LD_CALLBACK_RESUMED = 2, //opcode 6
    LD_CALLBACK_INVALID_SESSION = 3, //opcode 9
    LD_CALLBACK_CHANNEL_CREATE = 4,
    LD_CALLBACK_CHANNEL_UPDATE = 5,
    LD_CALLBACK_CHANNEL_DELETE = 6,
    LD_CALLBACK_CHANNEL_PINS_UPDATE = 7,
    LD_CALLBACK_GUILD_CREATE = 8,
    LD_CALLBACK_GUILD_UPDATE = 9,
    LD_CALLBACK_GUILD_DELETE = 10,
    LD_CALLBACK_GUILD_BAN_ADD = 11,
    LD_CALLBACK_GUILD_BAN_REMOVE = 12,
    LD_CALLBACK_GUILD_EMOJIS_UPDATE = 13,
    LD_CALLBACK_GUILD_INTEGRATIONS_UPDATE = 14,
    LD_CALLBACK_GUILD_MEMBER_ADD = 15,
    LD_CALLBACK_GUILD_MEMBER_REMOVE = 16,
    LD_CALLBACK_GUILD_MEMBER_UPDATE = 17,
    LD_CALLBACK_GUILD_MEMBERS_CHUNK = 18,
    LD_CALLBACK_GUILD_ROLE_CREATE = 19,
    LD_CALLBACK_GUILD_ROLE_UPDATE = 20,
    LD_CALLBACK_GUILD_ROLE_DELETE = 21,
    LD_CALLBACK_MESSAGE_CREATE = 22,
    LD_CALLBACK_MESSAGE_UPDATE = 23,
    LD_CALLBACK_MESSAGE_DELETE = 24,
    LD_CALLBACK_MESSAGE_DELETE_BULK = 25,
    LD_CALLBACK_MESSAGE_REACTION_ADD = 26,
    LD_CALLBACK_MESSAGE_REACTION_REMOVE = 27,
    LD_CALLBACK_MESSAGE_REACTION_REMOVE_ALL = 28,
    LD_CALLBACK_PRESENCE_UPDATE = 29,
    LD_CALLBACK_TYPING_START = 30,
    LD_CALLBACK_USER_UPDATE = 31,
    LD_CALLBACK_VOICE_STATE_UPDATE = 32,
    LD_CALLBACK_VOICE_SERVER_UPDATE = 33,
    LD_CALLBACK_WEBHOOKS_UPDATE = 34,
    LD_CALLBACK_USER = -1 //placeholder
};

enum ld_gateway_state {
    LD_GATEWAY_UNCONNECTED = 0,
    LD_GATEWAY_DISCONNECTED = 1,
    LD_GATEWAY_CONNECTING = 2,
    LD_GATEWAY_CONNECTED = 3
};

enum ld_gateway_opcode {
    LD_GATEWAY_OPCODE_UNKNOWN = -1,
    LD_GATEWAY_OPCODE_DISPATCH = 0,
    LD_GATEWAY_OPCODE_HEARTBEAT = 1,
    LD_GATEWAY_OPCODE_IDENTIFY = 2,
    LD_GATEWAY_OPCODE_PRESENCE = 3,
    LD_GATEWAY_OPCODE_VOICE_STATE = 4,
    LD_GATEWAY_OPCODE_VOICE_PING = 5,
    LD_GATEWAY_OPCODE_RESUME = 6,
    LD_GATEWAY_OPCODE_RECONNECT = 7,
    LD_GATEWAY_OPCODE_REQUEST_MEMBERS = 8,
    LD_GATEWAY_OPCODE_INVALIDATE_SESSION = 9,
    LD_GATEWAY_OPCODE_HELLO = 10,
    LD_GATEWAY_OPCODE_HEARTBEAT_ACK = 11,
    LD_GATEWAY_OPCODE_GUILD_SYNC = 12
};

enum ld_gateway_payloadtype {
    LD_GATEWAY_OP = 0,
    LD_GATEWAY_D = 1,
    LD_GATEWAY_T = 2,
    LD_GATEWAY_S = 3,
    LD_GATEWAY_UNKNOWN = 100
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
    enum ld_gateway_state gateway_state;
    int shards;
    CURLM *curl_multi_handle;
    struct lws_context *lws_context;
    struct lws *lws_wsi;
    int (*user_callback)
            (struct ld_context *context,
             enum ld_callback_reason reason,
             const char *data, int len);
    unsigned int heartbeat_interval; //always in ms
    int last_seq; //last sequence number received in the gateway
    void *gateway_queue;
    int close_code;
    int heartbeat; //0 for don't send, 1 for send
    unsigned long last_hb;
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
 * checks if the heartbeat timer is up
 */
int ld_service(struct ld_context *context, int timeout);

/*
 * starts a fresh gateway connection
 * will:
 *  start a fresh websocket connection
 *  parse HELLO payload
 *  create & send IDENTIFY payload
 *  start timekeeping for heartbeats
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

/*
 * takes a gateway payload and parses it
 * returns 0 on success
 * returns 1 on jansson (JSON parsing) error
 */
int ld_gateway_payload_parser(struct ld_context *context, char *in, size_t len);

/*
 * creates a gateway payload with four json fields
 */
json_t *ld_json_create_payload(struct ld_context *context, json_t *op, json_t *d, json_t *t, json_t *s);
#endif