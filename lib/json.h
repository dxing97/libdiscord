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
 * presence json object
 */
struct ld_json_presence {
    char *game;
    enum ld_presence_game_type game_type;
    enum ld_presence_status_type status_type;
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
    int shard;
    struct ld_json_presence *presence;
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

};

struct ld_json_embed_video {

};

struct ld_json_embed_provider {

};

struct ld_json_embed_author {

};

struct ld_json_embed_field {

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

struct ld_json_reaction {

};

struct ld_json_message_activity {

};

struct ld_json_message_application {

};

struct ld_json_message {
    LD_SNOWFLAKE id;
    LD_SNOWFLAKE channel_id;
    struct ld_json_user *author;
    char *content;
    char *timestamp; //ISO 8601 formatted string
    char *edited_timestamp;
    int tts; //boolean
    int mention_everyone; //boolean
    struct ld_json_user *mentions[]; //array of user objects. NOTE: last pointer in array is a null pointer
    struct ld_json_role *mention_roles[];
    struct ld_json_attachemnt *attachments[];
    struct ld_json_embed *embeds[];
    struct ld_json_reaction *reactions[];
    LD_SNOWFLAKE webhook_id;
    int type;
    struct ld_json_message_activity *activity;
    struct ld_json_message_application *application;
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
char *ld_snowflake_num2str();

#endif //LIBDISCORD_JSON_H
