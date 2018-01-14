#ifndef LIBDISCORD_0_3_LIBRARY_H
#define LIBDISCORD_0_3_LIBRARY_H

#include <curl/curl.h>
#include <libwebsockets.h>
#include <jansson.h>
#include "libdiscord_config.h"
#include "log.h"

/*
 * right now there are only callbacks for a small number of gateway events
 * More will be added eventually
 */
enum ld_callback_reason {
    LD_CALLBACK_UNKNOWN = -1, //placeholder

    /* payload opcodes */
    LD_CALLBACK_HELLO = 0, //opcode 10
    LD_CALLBACK_RESUMED = 2, //opcode 6
    LD_CALLBACK_INVALID_SESSION = 3, //opcode 9

    /* Dispatches */
    LD_CALLBACK_READY = 1, //dispatch (opcode 0)
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

    /* websocket specifis */
    LD_CALLBACK_WS_ESTABLISHED = 35, //websocket connection established and ready to rx/tx
    LD_CALLBACK_WS_CONNECTION_ERROR = 36 //error connecting to the gateway:

};

/*
 * connected: everything's normal
 * connecting: still working out the details
 * unconnected: we're not connected and we can start a fresh connection
 */
enum ld_gateway_state {
    LD_GATEWAY_UNCONNECTED = 0,
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

enum ld_gateway_disconnect_reason {
    LD_GATEWAY_DISCONNECT_NULL = 0
};

enum ld_presence_game_type {
    LD_PRESENCE_PLAYING = 0,
    LD_PRESENCE_STREAMING = 1,
    LD_PRESENCE_LISTENING = 2,
    LD_PRESENCE_WATCHING = 3
};

enum ld_presence_status_type {
    LD_PRESENCE_IDLE = 0,
    LD_PRESENCE_DND = 1,
    LD_PRESENCE_ONLINE = 2,
    LD_PRESENCE_OFFLINE = 3
};

/*
 * gateway ringbuffer elements
 * contains payload to be send and metadata
 */
struct ld_gateway_payload {
    void *payload; //array of chars to send
    size_t len; //size of payload in bytes

};

struct ld_presence {
    char *game;
    enum ld_presence_game_type gametype;
    enum ld_presence_status_type statustype;
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
             void *data);
    unsigned int heartbeat_interval; //always in ms
    int last_seq; //last sequence number received in the gateway
    unsigned long last_hb;
    int hb_count; //increments for every sent heartbeat, decrements for every received HB_ACK
    struct lws_ring *gateway_ring;
    unsigned int close_code;
    char *gateway_rx_buffer;
    size_t gateway_rx_buffer_len;
    struct ld_presence presence;
    char *gateway_session_id;
    int gateway_bot_limit; //ratelimit reset amount
    int gateway_bot_remaining; //last ratelimit remaining value
    unsigned long gateway_bot_reset; //unix time for reset

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
    int (*user_callback)(struct ld_context *context, enum ld_callback_reason reason, void *data);
    size_t gateway_ringbuffer_size;
    struct ld_presence init_presence;
};

struct ld_dispatch {
    const char* name;
    enum ld_callback_reason cbk_reason;
    int (*dispatch_callback)(struct ld_context *context, json_t *data);
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

int _ld_get_gateway_bot(struct ld_context *context);

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

size_t _ld_curl_response_string(void *contents, size_t size, size_t nmemb, void *userptr);

int _ld_get_gateway(struct ld_context *context);

size_t ld_curl_header_parser(char *buffer, size_t size, size_t nitems, void *userdata);

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
 * lws user callback
 * picks out interesting callback reasons (like receive and writeable) and does stuff
 * responsible for connecting/disconnecting from the gateway, receiving payloads, and sending payloads
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
 * takes four json_t objects and creates a payload
 */
json_t *ld_json_create_payload(struct ld_context *context, json_t *op, json_t *d, json_t *t, json_t *s);

/*
 * callback for parsing READY dispatches
 * saves session_id for resuming
 */
int ld_dispatch_ready(struct ld_context *context, json_t *data);

/*
 * type: json string object for dispatch type
 * data: json object containing dispatch data
 * takes dispatch data from the gateway and generates user callbacks based on dispatch type
 * returns 0 on success
 * returns 1 on jansson (JSON parsing) error
 */
int ld_gateway_dispatch_parser(struct ld_context *context, json_t *type, json_t *data);

/*
 * queues a heartbeat in the gateway tx ringbuffer
 */
int ld_gateway_queue_heartbeat(struct ld_context *context);

/*
 * calls lws_context_destroy to close the ws connection
 * sets the gateway state to unconnected
 * calls ld_gateway_connect to reinitialize the connection to the gateway
 */
int ld_gateway_reconnect(struct ld_context *context);
#endif