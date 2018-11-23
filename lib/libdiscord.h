#ifndef LIBDISCORD_0_3_LIBRARY_H
#define LIBDISCORD_0_3_LIBRARY_H

#include <curl/curl.h>
#include <libwebsockets.h>
#include <jansson.h>
#include "libdiscord_config.h"
#include "log.h"
#include "json.h"



/*
 *
 * libdiscord header file
 *
 *
 */

/*
 * return values
 */
enum ldecode {
    LDE_OK = 0, //everything is OK
    LDE_ULFIUS = 1, //problem doing something with ulfius
    LDE_JSON = 2, //problem doing something with json manipulation/jansson
    LDE_CURL = 3, //problem with something involving curl
    LDE_MEM = 4, //problem with something involving memory allocation/deallocation
    LDE_MISSING_PARAM = 5, //missing parameters
    LDE_HB_ACKMISS = 6, //didn't recieve an ACK from gateway
    LDE_HB_RINGBUF_FULL = 7
};

/*
 * reasons included with user callback
 */
enum ld_callback_reason {
    LD_CALLBACK_UNKNOWN = -1, //placeholder

    /* payload opcodes */
    LD_CALLBACK_HELLO = 0, //opcode 10
    LD_CALLBACK_RESUMED = 2, //opcode 6
    LD_CALLBACK_INVALID_SESSION = 3, //opcode 9

    /* Dispatches (opcode 0 types)*/
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

    /* websocket specific */
    LD_CALLBACK_WS_ESTABLISHED = 35, //websocket connection established and ready to rx/tx
    LD_CALLBACK_WS_CONNECTION_ERROR = 36, //error while trying to connect to the gateway
    LD_CALLBACK_WS_PEER_CLOSE = 37
    // the gateway closed the connection. len contains the close code,
    // and data may or may not contain a string with context data

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

/*
 * enum for opcodes that can we recieved from the gateway. 
 */
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

/*
 * enums for the four possible fields inside the discord gateway
 * todo: refactor GATEWAY to PAYLOAD
 */
enum ld_gateway_payloadtype {
    LD_GATEWAY_OP = 0,
    LD_GATEWAY_D = 1,
    LD_GATEWAY_T = 2,
    LD_GATEWAY_S = 3,
    LD_GATEWAY_UNKNOWN = 100
};

/*
 * enums for the game types that can be set in the presence.
 * todo: move to json.h
 */
enum ld_presence_game_type {
    LD_PRESENCE_PLAYING = 0,
    LD_PRESENCE_STREAMING = 1,
    LD_PRESENCE_LISTENING = 2,
    LD_PRESENCE_WATCHING = 3
};

//struct _ld_json_presence;

/*
 * gateway ringbuffer elements
 * contains payload to be send and metadata
 */
struct ld_gateway_payload {
    void *payload; //array of chars to send
    size_t len; //size of payload in bytes

};

struct ld_gi;

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
    unsigned long log_level; //DEPRECIATED, use new functions in log.h
    char *gateway_url;
    char *gateway_bot_url;
    enum ld_gateway_state gateway_state;
    int shards;
    CURLM *curl_multi_handle;
    CURL *curl_handle;
    struct lws_context *lws_context;
    struct lws *lws_wsi;
    int (*user_callback)
            (struct ld_context *context, enum ld_callback_reason reason, void *data, int len);
    unsigned int heartbeat_interval; //always in ms
    int last_seq; //last sequence number received in the gateway
    unsigned long last_hb;
    int hb_count; //increments for every sent heartbeat, decrements for every received HB_ACK
    struct lws_ring *gateway_ring;
    unsigned int close_code;
    char *gateway_rx_buffer;
    size_t gateway_rx_buffer_len;
//    struct _ld_json_presence *presence;
    char *gateway_session_id;
    int gateway_bot_limit; //ratelimit reset amount
    int gateway_bot_remaining; //last ratelimit remaining value
    unsigned long gateway_bot_reset; //unix time for reset
    struct ld_gi **gi;
    int gi_count;
    struct ld_json_user *current_user;
};

/*
 * libdiscord gateway connection info
 * one info struct per gateway connection (i.e. one per shard)
 * created and destroyed with each shard
 * does NOT go away with disconnections: destroyed when a shard is closed
 * //todo: when is a shard closed?
 */
struct ld_gi {
    struct ld_context *parent_context; //pointer to the parent context
    void *user; //user defined pointer for user stuff //todo: add way of allocating *user and setting its size
    enum ld_gateway_state state; //connected, connecting, disconnected
    //todo: have seperate states for websocket connecting and gateway identifying
    int shardnum; //which shard this gateway connection refers to
    struct lws *lws_wsi; //lws wsi corresponding to this connection
    unsigned int hb_interval; //interval to send heartbeats at, in ms
    int last_seq; //last sequence number recieved using this connection
    int hb_count; //starts at 0, increment
    struct lws_ring *tx_ringbuffer; //lws ringbuffer used to queue payloads to be sent;
    unsigned int close_code;
    int session_valid; //0 - no resume, non-0 - resume
};

/*
 * info used to generate a context
 * includes:
 *  bot token
 *  user-defined pointer to anything, can be metadata about the bot (creator, version, etc.)
 *  libdiscord logging level (see ld_log_level), DEPRECIATED, use logging functions in log.h instead
 */
struct ld_context_info {
    char *bot_token;
//    unsigned long log_level;  //DEPRECIATED, use new functions in log.h
    int (*user_callback)(struct ld_context *context, enum ld_callback_reason reason, void *data, int len);
    size_t gateway_ringbuffer_size;
//    struct _ld_json_presence *init_presence;
};

struct ld_dispatch {
    const char* name;
    enum ld_callback_reason cbk_reason;
    int (*dispatch_callback)(struct ld_context *context, json_t *data);
};



/*
 * DEPRECIATED, USE FUNCTIONS IN log.h INSTEAD
 * logging functions for different levels
 * context is needed to determine enabled logging levels
 */

//void _ld_err(struct ld_context *context, const char *message, ...);
//void _ld_warn(struct ld_context *context, const char *message, ...);
//void _ld_info(struct ld_context *context, const char *message, ...);
//void _ld_note(struct ld_context *context, const char *message, ...);
//void _ld_dbug(struct ld_context *context, const char *message, ...);

/*
 * creates a context from user info
 * returns NULL if the info struct was malformed or missing things
 * allocates memory for the struct and internal components
 */
struct ld_context *ld_init_context(struct ld_context *context, struct ld_context_info *info);

/*
 * destroys context
 * frees inner components and the context itself
 */
void ld_destroy_context(struct ld_context *context);

/*
 * private function, makes a GET request to /gateway/bot and retrieves shard number and gateway URL/determines bot token
 * is invalid. Uses the ulfius blocking REST interface.
 */
int _ld_get_gateway_bot(struct ld_context *context);

/*
 * curl callback function used to read data returned from HTTP request
 */
size_t _ld_curl_response_string(void *contents, size_t size, size_t nmemb, void *userptr);

/*
 * private function, makes a GET request to /gateway and retrieves the gateway URL
 * used to determine if we can even connect to Discord, not _strictly_ nessecary
 * blocking
 */
int _ld_get_gateway(struct ld_context *context);

/*
 * curl callback function used to (currently) print out HTTP headers line by line for /gateway and /gateway/bot
 */
size_t ld_curl_header_parser(char *buffer, size_t size, size_t nitems, void *userdata);

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
 * returns 0 for OK
 * returns 1 for websocket ringbuffer error
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
 * tries to get the operating system name
 */
char *ld_get_os_name();



#include "REST.h"
#include "json.h"
#endif