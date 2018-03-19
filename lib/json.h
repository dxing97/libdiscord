//
// Created by dxing97 on 1/15/18.
//

#ifndef LIBDISCORD_JSON_H
#define LIBDISCORD_JSON_H

#include "libdiscord.h"
#include <jansson.h>

/*
 * snowflakes are 64 bit unsigned integers encoded in decimal in a string for maximum compatability with languages that
 * can't handle 64 bit integers
 *
 * something that libdiscord should check to see is that some architectures don't support numbers past a certain size
 * i.e. 32 bit only
 * how we handle that will be interesting
 */
typedef uint64_t LD_SNOWFLAKE;

/*
 * timestamp types are strings formatted in the ISO8601 format
 */
typedef char * TIMESTAMP;

enum ld_json_status_type {
    LD_PRESENCE_IDLE = 0,
    LD_PRESENCE_DND = 1,
    LD_PRESENCE_ONLINE = 2,
    LD_PRESENCE_OFFLINE = 3
};

struct ld_json_status_update {
//    struct ld_json_user *user; // no longer in the discord API documentation???
    LD_SNOWFLAKE *roles; //array of snowflakes
    struct ld_json_activity *game;
    LD_SNOWFLAKE guild_id;
    char *status;
};

/*
 * connection properties json object
 */
struct ld_json_identify_connection_properties {
    char *os;
    char *browser;
    char *device;
};

/*
 * identify json object
 * sent with opcode 2 payloads (IDENTIFY)
 *
 */
struct ld_json_identify {
    char *token;
    struct ld_json_identify_connection_properties *properties;
    int compress;
    int large_threshold;
    int shard[2];
    struct ld_json_status_update *status_update;
};

struct ld_json_party {
    char *id;
    int size[2];
};

struct ld_json_assets {
    char *large_image;
    char *large_text;
    char *small_image;
    char *small_text;
};

struct ld_json_activity {
    char *name;
    int type;
    char *url; //optional and nullable (double check this)
    struct ld_json_timestamps *timestamps;
    LD_SNOWFLAKE application_id;
    char *details;
    char *state;
    struct ld_json_party *party;
    struct ld_json_assets *assets;
};

struct ld_json_gateway_update_status {
    int since; //nullable
    struct ld_json_activity *game;
    char *status;
    int afk;
};

struct ld_json_user {
    LD_SNOWFLAKE id;
    char *username;
    char *discriminator; //4 digit tag
    char *avatar;
    int bot; //boolean
    int mfa_enabled; //boolean
    int verified; //boolean (email verification)
    char *email;
};

struct ld_json_role {
    LD_SNOWFLAKE id;
    char *name;
    int color; //integer representation of hex color code
    int hoist; //boolean
    int position;
    int permissions; //bitfield
    int managed; //boolean
    int mentionable;
};

struct ld_json_attachemnt {
    LD_SNOWFLAKE id;
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
    LD_SNOWFLAKE id;
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
    LD_SNOWFLAKE id;
    char *cover_image;
    char *description;
    char *icon;
    char *name;
};

struct ld_json_message {
    LD_SNOWFLAKE id;
    LD_SNOWFLAKE channel_id;
    struct ld_json_user *author;
    char *content;
    TIMESTAMP timestamp; //ISO 8601 formatted string
    TIMESTAMP edited_timestamp; //ISO 8601 formatted string
    int tts; //boolean
    int mention_everyone; //boolean
    struct ld_json_user **mentions; //array of user objects. NOTE: last pointer in array is a null pointer
    struct ld_json_role **mention_roles;
    struct ld_json_attachemnt **attachments;
    struct ld_json_embed **embeds;
    struct ld_json_reaction **reactions;
    LD_SNOWFLAKE webhook_id;
    int type;
    struct ld_json_message_activity *activity;
    struct ld_json_message_application *application;
};

struct ld_json_overwrite {
    LD_SNOWFLAKE id;
    char *type;
    int allow;
    int deny;
};

struct ld_json_attachment {
    LD_SNOWFLAKE id;
    char *filename;
    int size;
    char *url;
    char *proxy_url;
    int height;
    int width;
};

struct ld_json_channel {
    LD_SNOWFLAKE id;
    int type;
    LD_SNOWFLAKE guild_id;
    int position;
    struct ld_json_overwrite **permission_overwrites;
    char *name;
    char *topic;
    int nsfw; //boolean
    LD_SNOWFLAKE last_message_id;
    int bitrate;
    int user_limit;
    struct ld_json_user **recipients;
    char *icon;
    LD_SNOWFLAKE owner_id;
    LD_SNOWFLAKE application_id;
    LD_SNOWFLAKE parent_id;
    TIMESTAMP last_pin_timestamp; //ISO8601 formatted string
};

struct ld_json_voice_state {
    LD_SNOWFLAKE guild_id;
    LD_SNOWFLAKE channel_id;
    LD_SNOWFLAKE user_id;
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
    LD_SNOWFLAKE *roles; //array of ints
    TIMESTAMP joined_at; //ISO8601 formatted timestamp;
    int deaf; //boolean
    int mute; //boolean
};

/*
 * Depreciated: DO NOT USE
 */
struct _ld_json_presence {
    char *game;
    enum ld_presence_game_type game_type;
    enum ld_json_status_type status_type;
};



struct ld_json_guild {
    LD_SNOWFLAKE id;
    char *name;
    char *icon;
    char *splash;
    int owner;
    LD_SNOWFLAKE owner_id;
    int permissions;
    char *region;
    LD_SNOWFLAKE afk_channel_id;
    int afk_timeout;
    int embed_enabled; //bool
    LD_SNOWFLAKE embed_channel_id;
    int verification_level;
    int default_message_notifications;
    int explicit_content_filter;
    struct ld_json_role **roles;
    struct ld_json_emoji **emojis;
    char **features; //array of strings
    int mfa_level;
    LD_SNOWFLAKE application_id;
    int widget_enabled; //boolean
    LD_SNOWFLAKE widget_channel_id;
    LD_SNOWFLAKE system_channel_id;
    TIMESTAMP joined_at; //ISO8601 timestamp formatted string
    int large; //boolean
    int unavailable; //boolean
    int member_count;
    struct ld_json_voice_state **voice_states;
    struct ld_json_guild_member **members;
    struct ld_json_channel **channels;
    struct ld_json_presence_update **presences;
};

struct ld_json_account {
    LD_SNOWFLAKE id; //discord API documentation lists this as a string type
    char *name;
};

struct ld_json_ban {
    char *reason;
    struct ld_json_user *user;
};

struct ld_json_integration {
    LD_SNOWFLAKE id;
    char *name;
    char *type;
    int enabled; //boolean
    int syncing; //boolean
    LD_SNOWFLAKE role_id;
    int expire_behavior;
    int expire_grace_period;
    struct ld_json_user *user;
    struct ld_json_account *account;
    char *synced_at;
};

/*
 * takes four json_t objects and creates a payload
 */
json_t *ld_json_create_payload(json_t *op, json_t *d, json_t *t, json_t *s);

/*
 * creates a Discord message object
 */
json_t *ld_json_create_message();

//todo: snowflake conversion functions
uint64_t ld_snowflake_str2num();
char *ld_snowflake_num2str(LD_SNOWFLAKE flake);

json_t *ld_json_dump_activity(struct ld_json_activity *activity);

json_t *ld_json_dump_user(struct ld_json_user *user);

json_t *ld_json_dump_status_update(struct ld_json_status_update *status_update);

json_t *ld_json_dump_identify_connection_properties(struct ld_json_identify_connection_properties *properties);

json_t *ld_json_dump_identify(struct ld_json_identify *identify);

const char *ld_json_status2str(enum ld_json_status_type type);

#endif //LIBDISCORD_JSON_H
