//
// Created by dxing97 on 1/15/18.
//

#ifndef LIBDISCORD_JSON_H
#define LIBDISCORD_JSON_H

#include "libdiscord.h"
#include <jansson.h>

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
    uint64_t application_id; //snowflake
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
