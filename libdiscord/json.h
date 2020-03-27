/** @file */

#ifndef LIBDISCORD_JSON_H
#define LIBDISCORD_JSON_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <libdiscord.h>
#include <jansson.h>
#include <time.h>

#include "REST.h"

/*
 *
 * native type: char * (string), uint64_t, etc
 * READ:
 * string -> struct
 *
 * SAVE:
 * struct -> string
 * (native type) -> string
 *
 * LOAD:
 * string -> json_t
 * string -> (native type)
 *
 * PACK:
 * json_t -> struct
 *
 * UNPACK:
 * struct -> json_t
 *
 * DUMP:
 * json_t -> string
 *
 * VALID:
 * checks if a struct's values conform to expected values
 *
 *
 *
 * snowflake: uint64_t
 *
 * timestamps: strings
 * example: 2019-02-09T21:31:47.083000+00:00
 */

    //forward declarations
enum ld_gateway_opcode;
enum ld_dispatch_event;

enum ld_presence_activity_type;
enum ld_json_status_type;
enum ld_activity_flags;


//library-specific
struct ld_timestamp;

// JSONs encapsulated into structs
struct ld_json_snowflake;
struct ld_json_status_update;
struct ld_json_identify_connection_properties;
struct ld_json_identify;
struct ld_json_party;
struct ld_json_assets;
struct ld_json_activity;
struct ld_json_secrets;
struct ld_json_gateway_update_status;
struct ld_json_user;
struct ld_json_role;
struct ld_json_attachemnt;
struct ld_json_embed_footer;
struct ld_json_embed_image;
struct ld_json_embed_thumbnail;
struct ld_json_embed_video;
struct ld_json_embed_provider;
struct ld_json_embed_author;
struct ld_json_embed_field;
struct ld_json_embed;
struct ld_json_emoji;
struct ld_json_reaction;
struct ld_json_message_activity;
struct ld_json_message_application;
struct ld_json_message;
struct ld_json_overwrite;
struct ld_json_attachment;
struct ld_json_channel;
struct ld_json_voice_state;
struct ld_json_guild_member;
struct ld_json_guild;
struct ld_json_account;
struct ld_json_ban;
struct ld_json_integration;

struct ld_json_getgateway;
struct ld_json_getgateway_bot;
struct ld_json_getgateway_bot_sessionstartlimit;

struct ld_json_websocket_payload;
struct ld_json_resume;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
#pragma mark Enumeration declarations
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief activity type
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#activity-object-activity-types" Discord API Documentation</a>
 */
enum ld_presence_activity_type {
    LD_PRESENCE_ACTIVITY_PLAYING = 0,
    LD_PRESENCE_ACTIVITY_STREAMING = 1,
    LD_PRESENCE_ACTIVITY_LISTENING = 2
//    LD_PRESENCE_ACTIVITY_WATCHING = 3
};


/**
 * @brief Status enum type
 *
 * enum integer values are used internally, strings are sent in payloads
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#update-status-status-types" Discord API Documentation</a>
 */
enum ld_json_status_type {
    LD_PRESENCE_IDLE = 0, ///< "idle"
    LD_PRESENCE_DND = 1, ///< "dnd"
    LD_PRESENCE_ONLINE = 2, ///< "online"
    LD_PRESENCE_OFFLINE = 3, ///< "offline"
    LD_PRESENCE_INVISIBLE = 4 ///< "invisible"
};

/**
 * @brief Activity flags enum type
 *
 * enum values are meant to be OR'd together
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#activity-object-activity-flags" Discord API Documentation</a>
 */
enum ld_activity_flags {
    LD_ACTIVITY_FLAG_INSTANCE = 1 << 0,
    LD_ACTIVITY_FLAG_JOIN = 1 << 1,
    LD_ACTIVITY_FLAG_SPECTATE = 1 << 2,
    LD_ACTIVITY_FLAG_JOIN_REQUEST = 1 << 3,
    LD_ACTIVITY_FLAG_SYNC = 1 << 4,
    LD_ACTIVITY_FLAG_PLAY = 1 << 5

};

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
#pragma mark Struct declarations
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


/**
 * @brief timestamp struct
 * used for timestamp fields in events
 * example string: 2019-02-09T21:31:47.083000+00:00
 */
struct ld_timestamp {
    char * iso_str; ///< raw ISO8601 formatted string
    uint64_t unix_epoch; ///< unix epoch in seconds
    uint64_t discord_epoch; ///< milliseconds since first second of 2015
};

/**
 * @brief decoded snowflake struct
 *
 * <a href="https://discordapp.com/developers/docs/reference#snowflakes" Discord API Documentation</a>
 */
struct ld_json_snowflake {
    uint64_t timestamp; ///< milliseconds since the first second of 2015, aka Discord epoch
    unsigned long long worker_id; ///< internal worker ID
    unsigned long long process_id; ///< internal process ID
    unsigned long long increment; ///< incremented for every generated snowflake on that process

    uint64_t unix_timestamp; ///< unix timestamp in milliseconds
};

/**
 * @brief Status update
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#update-status-gateway-status-update-structure" Discord API Documentation </a>
 */
struct ld_json_status_update {
//    struct ld_json_user *user; // no longer in the discord API documentation
    uint64_t *roles; ///< array of snowflakes, no longer in the discord API documentation?
    struct ld_json_activity *game; ///< nullable field
    uint64_t guild_id; // no longer in the discord API documentation?
    enum ld_json_status_type status;
    int since; ///< nullable field
    int afk; ///< boolean
};

/**
 * @brief Identify connection properties json object
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#identify-identify-connection-properties" Discord API Documentation </a>
 */
struct ld_json_identify_connection_properties {
    char *os;
    char *browser;
    char *device;
};

/**
 * @brief Identify json object
 * sent with opcode 2 payloads (IDENTIFY)
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#identify-identify-structure" Discord API Documentation </a>
 *
 */
struct ld_json_identify {
    char *token; ///< auth token
    struct ld_json_identify_connection_properties *properties;
    int compress; ///< optional field
    int large_threshold; ///< optional field
    int shard[2]; ///< [shard id, number of shards]
    struct ld_json_status_update *status_update; ///< optional field
};

/**
 * @brief Party json object
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#activity-object-activity-party" Discord API Documentation </a>
 *
 */
struct ld_json_party {
    char *id; ///< optional
    int size[2]; ///< optional
};

/**
 * @brief Activity assets json object
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#activity-object-activity-assets" Discord API Documentation </a>
 *
 */
struct ld_json_assets {
    char *large_image; ///< optional
    char *large_text; ///< optional
    char *small_image; ///< optional
    char *small_text; ///< optional
};

/**
 * @brief Activity json object
 * bots may only send name, type, and optionally url
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#activity-object-activity-structure" Discord API Documentation </a>
 *
 */
struct ld_json_activity {
    char *name;
    enum ld_presence_activity_type type;
    char *url; ///< optional and nullable
    struct ld_json_timestamps *timestamps; ///< optional
    uint64_t application_id; ///< optional
    char *details; ///< optional and nullable
    char *state; ///< optional and nullable
    struct ld_json_party *party; ///< optional
    struct ld_json_assets *assets; ///< optional
    struct ld_json_secrets *secrets; ///< optional
    int flags; ///optional, OR'd activity flag enums
};

/**
 * @brief Activity secrets json object
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#activity-object-activity-secrets"> Discord API Documentation </a>
 *
 */
struct ld_json_secrets {
    char *join; ///< optional
    char *spectate; ///< optional
    char *match; ///< optional
};

/**
 * @brief Gateway status update json object
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#update-status-gateway-status-update-structure"> Discord API Documentation </a>
 */

struct ld_json_gateway_update_status {
    int since; ///< nullable
    struct ld_json_activity *game; ///< nullable
    char *status;
    int afk;
};

/**
 *
 */
struct ld_json_user {
    uint64_t id;
    char *username;
    char *discriminator; //4 digit tag
    char *avatar;
    int bot; //boolean
    int mfa_enabled; //boolean
    char *locale;
    int verified; //boolean (email verification)
    char *email;
};

struct ld_json_role {
    uint64_t id;
    char *name;
    int color; //integer representation of hex color code
    int hoist; //boolean
    int position;
    int permissions; //bitfield
    int managed; //boolean
    int mentionable;
};

struct ld_json_attachemnt {
    uint64_t id;
    char *filename;
    int size;
    char *url;
    char *proxy_url;
    int height;
    int width;
};

struct ld_json_embed_footer {
    char *text;
    char *icon_url;
    char *proxy_icon_url;
};

struct ld_json_embed_image {
    char *url;
    char *proxy_url;
    int height;
    int width;
};

struct ld_json_embed_thumbnail {
    char *url;
    char *proxy_url;
    int height;
    int width;
};

struct ld_json_embed_video {
    char *url;
    int height;
    int width;
};

struct ld_json_embed_provider {
    char *name;
    char *url;
};

struct ld_json_embed_author {
    char *name;
    char *url;
    char *icon_url;
    char *proxy_icon_url;
};

struct ld_json_embed_field {
    char *name;
    char *value;
    int _inline; //bool
};

struct ld_json_embed {
    char *title;
    char *type;
    char *description;
    char *url;
    char *timestamp; //ISO8601 formatted string
    int color;
    struct ld_json_embed_footer *footer;
    struct ld_json_embed_image *image;
    struct ld_json_embed_thumbnail *thumbnail;
    struct ld_json_embed_video *video;
    struct ld_json_embed_provider *provider;
    struct ld_json_embed_author *author;
    struct ld_json_embed_field *fields[];
};

struct ld_json_emoji {
    uint64_t id;
    char *name;
    struct ld_json_role **roles; //pointer to array of pointers
    struct ld_json_user *user;
    int require_colons;
    int managed;
    int animated;
};

struct ld_json_reaction {
    int count;
    int me; //boolean
    struct ld_json_emoji *emoji; //from the documentation: "partion emoji object type"
};

struct ld_json_message_activity {
    int type;
    char *party_id;
};

struct ld_json_message_application {
    uint64_t id;
    char *cover_image;
    char *description;
    char *icon;
    char *name;
};

struct ld_json_message {
    uint64_t id;
    uint64_t channel_id;
    struct ld_json_user *author;
    char *content;
    char * timestamp; //ISO 8601 formatted string
    char * edited_timestamp; //ISO 8601 formatted string
    int tts; //boolean
    int mention_everyone; //boolean
    struct ld_json_user **mentions; //array of user objects. NOTE: last pointer in array is a null pointer
    struct ld_json_role **mention_roles;
    struct ld_json_attachemnt **attachments;
    struct ld_json_embed **embeds;
    struct ld_json_reaction **reactions;
    uint64_t webhook_id;
    int type;
    struct ld_json_message_activity *activity;
    struct ld_json_message_application *application;
};

struct ld_json_overwrite {
    uint64_t id;
    char *type;
    int allow;
    int deny;
};

struct ld_json_attachment {
    uint64_t id;
    char *filename;
    int size;
    char *url;
    char *proxy_url;
    int height;
    int width;
};

struct ld_json_channel {
    uint64_t id;
    int type;
    uint64_t guild_id;
    int position;
    struct ld_json_overwrite **permission_overwrites;
    char *name;
    char *topic;
    int nsfw; //boolean
    uint64_t last_message_id;
    int bitrate;
    int user_limit;
    struct ld_json_user **recipients;
    char *icon;
    uint64_t owner_id;
    uint64_t application_id;
    uint64_t parent_id;
    char * last_pin_timestamp; //ISO8601 formatted string
};

struct ld_json_voice_state {
    uint64_t guild_id;
    uint64_t channel_id;
    uint64_t user_id;
    char *session_id; //documentation lists as string type
    int deaf; //boolean
    int mute; //boolean
    int self_deaf; //boolean
    int self_mute; //boolean
    int supress; //boolean
};

struct ld_json_guild_member {
    struct ld_json_user *user;
    char *nick;
    uint64_t *roles; //array of ints
    char * joined_at; //ISO8601 formatted timestamp;
    int deaf; //boolean
    int mute; //boolean
};

struct ld_json_guild {
    uint64_t id;
    char *name;
    char *icon;
    char *splash;
    int owner;
    uint64_t owner_id;
    int permissions;
    char *region;
    uint64_t afk_channel_id;
    int afk_timeout;
    int embed_enabled; //bool
    uint64_t embed_channel_id;
    int verification_level;
    int default_message_notifications;
    int explicit_content_filter;
    struct ld_json_role **roles;
    struct ld_json_emoji **emojis;
    char **features; //array of strings
    int mfa_level;
    uint64_t application_id;
    int widget_enabled; //boolean
    uint64_t widget_channel_id;
    uint64_t system_channel_id;
    char * joined_at; //ISO8601 timestamp formatted string
    int large; //boolean
    int unavailable; //boolean
    int member_count;
    struct ld_json_voice_state **voice_states;
    struct ld_json_guild_member **members;
    struct ld_json_channel **channels;
    struct ld_json_status_update **presences;
};

struct ld_json_account {
    uint64_t id; //discord API documentation lists this as a string type
    char *name;
};

struct ld_json_ban {
    char *reason;
    struct ld_json_user *user;
};

struct ld_json_integration {
    uint64_t id;
    char *name;
    char *type;
    int enabled; //boolean
    int syncing; //boolean
    uint64_t role_id;
    int expire_behavior;
    int expire_grace_period;
    struct ld_json_user *user;
    struct ld_json_account *account;
    char *synced_at;
};

struct ld_json_getgateway {
    char *url; ///< /gateway url
};

struct ld_json_getgateway_bot {
    char *url;
    int shards;
    struct ld_json_getgateway_bot_sessionstartlimit *limits;
};


struct ld_json_getgateway_bot_sessionstartlimit {
    int total;
    int remaining;
    int reset_after;
};

/**
 * @brief Gateway payload
 *
 * @todo how to identify struct type?
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#payloads"> Discord API Documentation </a>
 */
struct ld_json_websocket_payload {
    enum ld_gateway_opcode op;///< opcode
    void *d; ///< data, pointer to some struct. must be present for this to be valid
    int s; ///< seq number, only on opcode 0
//    char *t; ///< event name,  only on opcode 0
    enum ld_dispatch_event t; ///< event name, only used for opcode 0

};

/**
 * @brief Gateway resume data field
 *
 * <a href="https://discordapp.com/developers/docs/topics/gateway#resume"> Discord API Documentation </a>
 */
struct ld_json_resume {
    char *token; ///< user/bot token
    char *session_id;
    int seq; ///< sequence number
};

/**
 * @brief struct equivalent of "[]"
 * is this a good solution?
 */
struct ld_json_empty_array {

};

//// functions

/*
 * takes four json_t objects and creates a payload
 */
json_t *ld_json_create_payload(json_t *op, json_t *d, json_t *t, json_t *s);

/*
 * creates a Discord message object
 */
json_t *ld_json_create_message();


uint64_t ld_str2snowflake();
char * ld_snowflake2str(uint64_t flake);
json_t *ld_json_dump_activity(struct ld_json_activity *activity);
int ld_json_load_user(struct ld_json_user *new_user, json_t *user);
json_t *ld_json_unpack_user(struct ld_json_user *user);
json_t *ld_json_unpack_status_update(struct ld_json_status_update *status_update);
json_t *ld_json_unpack_identify_connection_properties(struct ld_json_identify_connection_properties *properties);
json_t *ld_json_unpack_identify(struct ld_json_identify *identify);
const char *ld_json_status2str(enum ld_json_status_type type);
int ld_json_message_init(struct ld_json_message *message);
int ld_json_message_cleanup(struct ld_json_message *message);
int *ld_json_pack_message(struct ld_json_message *new_message, json_t *message);
int ld_json_pack_snowflake(struct ld_json_snowflake *new_flake, uint64_t snowflake);
int ld_json_read_timestamp(struct ld_timestamp *new_timestamp, char *timestamp);


/**
 * @brief Internal json dump function used for most ld_json_dump_* functions
 * @param out Pointer to save string to, must be freed by caller
 * @param in json_t containing json data
 * @return LDS enum, LDS_OK on success, LDS_JSON_* on error
 */
ld_status _ld_json_dump_all(char **out, json_t *in, const char *caller);

ld_status ld_json_unpack_resume(json_t *out, struct ld_json_resume *resume);
ld_status ld_json_dump_resume(char **out, json_t *resume);
ld_status ld_json_save_resume(char **out, struct ld_json_resume *resume);

/**
 * @brief Checks payload struct against Discord's field rules
 * @param payload pointer to filled payload. Cannot be null.
 * @return LDS_OK if valid, LD_JSON_* error otherwise
 */
ld_status ld_json_payload_valid(struct ld_json_websocket_payload *payload);
ld_status ld_json_unpack_payload(json_t *out, struct ld_json_websocket_payload *payload);
ld_status ld_json_dump_payload(char **out, json_t *payload);
ld_status ld_json_save_payload(char **out, const struct ld_json_websocket_payload *payload);

#endif //LIBDISCORD_JSON_H
