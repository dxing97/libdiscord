//
// Created by dxing97 on 1/15/18.
//
//#include <jansson.h>
#include "json.h"
#include "log.h"

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

json_t *ld_json_dump_presence(struct ld_json_presence *presence) {
    return NULL;
}

json_t *ld_json_dump_identify_connection_properties(struct ld_json_identify_connection_properties *properties) {
    return NULL;
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

    if(identify->presence != NULL) {
        //presence is optional
        json_object_set(ident, "presence", ld_json_dump_presence(identify->presence));
    }

    return ident;
}