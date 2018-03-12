//
// Created by dxing97 on 1/15/18.
//
//#include <jansson.h>
#include "json.h"
#include "log.h"
#include <stdio.h>

char *ld_snowflake_num2str(LD_SNOWFLAKE flake) {
    char *tmp;
    tmp = malloc(sizeof(char)*128);
    snprintf(tmp, 127, "%lu", (uint64_t) flake);
    return tmp;
}

json_t *ld_json_create_payload(json_t *op, json_t *d, json_t *t, json_t *s) {
    json_t *payload;
    payload = json_object();
    int ret;
    ret = json_object_set_new(payload, "op", op);
    if(ret != 0) {
        ld_warning("couldn't set opcode in new payload");
        return NULL;
    }
    if(d != NULL){ //can be null for heartbeat ACKs
        ret = json_object_set_new(payload, "d", d);
        if(ret != 0) {
            ld_warning("couldn't set data in new payload");
            return NULL;
        }
    }

    if(t != NULL) {
        ret = json_object_set_new(payload, "t", t);
        if(ret != 0) {
            ld_warning("couldn't set type in new payload");
            return NULL;
        }
    }
    if(s != NULL) {
        ret = json_object_set_new(payload, "s", s);
        if(ret != 0) {
            ld_warning("couldn't set sequence number in new payload");
            return NULL;
        }
    }


    return payload;
}

json_t *ld_json_dump_activity(struct ld_json_activity *activity) {
    return NULL;
}

json_t *ld_json_dump_user(struct ld_json_user *user) {

    return NULL;
}

json_t *ld_json_dump_status_update(struct ld_json_status_update *status_update) {
    json_t *su = NULL;
    su = json_object();
    int i;

    char tmp[128];
    snprintf(tmp, 127, "%lu", (uint64_t) status_update->guild_id);

    json_object_set(su, "user", json_string(ld_snowflake_num2str(status_update->guild_id)));

    json_t *roles = json_array();

    for(i = 0; i < strlen((char *) status_update->roles); i++) {
        json_array_append_new(roles, json_string(ld_snowflake_num2str(status_update->roles[i])));
    }
    json_object_set(su, "roles", roles);

    if(status_update->game != NULL) {
        json_object_set(su, "game", ld_json_dump_activity(status_update->game));
    }

    if(status_update->status != NULL) {
        json_object_set(su, "status", json_string(status_update->status));
    }

    return su;
}

json_t *ld_json_dump_identify_connection_properties(
        struct ld_json_identify_connection_properties *properties) {
    json_t *p = NULL;
    p = json_object();

    if(p == NULL){
        ld_error("couldn't allocte json object for identify connection properties");
        return NULL;
    }

    if(properties->os == NULL) {
        json_object_set(p, "$os", json_string("unknown"));
    } else {
        json_object_set(p, "$os", json_string(properties->os));
    }

    if(properties->device == NULL) {
        json_object_set(p, "$device", json_string(LD_LIBNAME));
    } else {
        json_object_set(p, "$device", json_string(properties->device));
    }

    if(properties->browser == NULL) {
        json_object_set(p, "$browser", json_string(LD_LIBNAME));
    } else {
        json_object_set(p, "$browser", json_string(properties->browser));
    }

    return p;
}

json_t *ld_json_dump_identify(struct ld_json_identify *identify) {
    json_t *ident = NULL;
//    json_error_t error;
    //todo: add error checking

    ident = json_object();
    if(identify->token == NULL) {
        ld_error("ld_json_dump_identify: token is NULL");
        return NULL;
    }
    json_object_set(ident, "token", json_string(identify->token));

    if(identify->properties == NULL) {
        ld_error("ld_json_dump_identify: identify connection properties is NULL");
        return NULL;
    }
    json_object_set(ident, "properties", ld_json_dump_identify_connection_properties(identify->properties));

    json_object_set(ident, "compress", json_boolean(identify->compress));

    if((identify->large_threshold > 250) || (identify->large_threshold < 50)) {
        ld_warning("ld_json_dump_identify: large_theshold is out of expected range");
    }
    json_object_set(ident, "large_threshold", json_integer(identify->large_threshold));

    json_object_set(ident, "shard", json_integer(identify->shard)); //todo: this is wrong, should be json array

    if(identify->status_update != NULL) {
        //presence is optional
        json_object_set(ident, "presence", ld_json_dump_status_update(identify->status_update));
    }

    return ident;
}

const char *ld_json_status2str(enum ld_json_status_type type) {
    switch(type) {
        case LD_PRESENCE_IDLE:
            return "idle";
        case LD_PRESENCE_DND:
            return "dnd";
        case LD_PRESENCE_ONLINE:
            return "online";
        case LD_PRESENCE_OFFLINE:
            return "offline";
    }
    return NULL;
}


