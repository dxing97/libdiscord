//
// Created by dxing97 on 1/15/18.
//
//#include <jansson.h>
#include "json.h"
#include "log.h"
#include <stdio.h>




char * ld_snowflake2str(uint64_t flake) {
    char *tmp;
    tmp = malloc(sizeof(char)*128);
    snprintf(tmp, 127, "%llu", (unsigned long long) flake);
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

int ld_json_user_init(struct ld_json_user *user) {
    user->username = NULL;
    user->discriminator = NULL;
    user->avatar = NULL;
    user->locale = NULL;
    return 0;
}

int ld_json_user_cleanup(struct ld_json_user *user) {
    if(user->username != NULL) {
        free(user->username);
    }
    if(user->discriminator != NULL) {
        free(user->discriminator);
    }
    if(user->avatar != NULL) {
        free(user->avatar);
    }
    if(user->locale != NULL) {
        free(user->locale);
    }
    return 0;
}

int ld_json_load_user(struct ld_json_user *new_user, json_t *user) {
    json_t *value;
    int bool;
    const char *key;
    const char *tmp;

    json_object_foreach(user, key, value) {
        if(strcmp(key, "id") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_warning("ld_json_load_user: couldn't get user id");
            } else {
                new_user->id = strtoull(tmp, NULL, 10);
            }
        }

        if(strcmp(key, "username") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_warning("ld_json_load_user: couldn't get username");
            } else {
                new_user->username = strdup(tmp);
            }
        }

        if(strcmp(key, "discriminator") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_warning("ld_json_load_user: couldn't get user discriminator");
            } else {
                new_user->discriminator = strdup(tmp);
            }
        }

        if(strcmp(key, "avatar") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_warning("ld_json_load_user: couldn't get user avatar");
            } else {
                new_user->avatar = strdup(tmp);
            }
        }

        if(strcmp(key, "bot") == 0) {
            bool = json_boolean_value(value);
            new_user->bot = bool;
        }

        if(strcmp(key, "mfa_enabled") == 0) {
            bool = json_boolean_value(value);
            new_user->mfa_enabled = bool;
        }

        if(strcmp(key, "locale") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_warning("ld_json_load_user: couldn't get user locale");
                new_user->locale = NULL;
            } else {
                new_user->locale = strdup(tmp);
            }
        }

        if(strcmp(key, "verified") == 0) {
            bool = json_boolean_value(value);
            new_user->verified = bool;
        }

        if(strcmp(key, "email") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_notice("ld_json_load_user: couldn't get user email");
            } else {
                new_user->email = strdup(tmp);
            }
        }


    }

    return 0;
}

json_t *ld_json_unpack_user(struct ld_json_user *user) {

    return NULL;
}


json_t *ld_json_unpack_status_update(struct ld_json_status_update *status_update) {
    json_t *su = NULL;
    su = json_object();
    int i;

    char tmp[128];
    snprintf(tmp, 127, "%llu", (unsigned long long) status_update->guild_id);

    json_object_set(su, "user", json_string(ld_snowflake2str(status_update->guild_id)));

    json_t *roles = json_array();


    if(status_update->roles != NULL) {
        ld_debug("role: %d", *status_update->roles);
        for (i = 0; i < strlen((char *) status_update->roles); i++) {
            json_array_append_new(roles, json_string(ld_snowflake2str(status_update->roles[i])));
        }
        json_object_set(su, "roles", roles);
    }

    if(status_update->game != NULL) {
        json_object_set(su, "game", ld_json_dump_activity(status_update->game));
    }

    json_object_set(su, "status", json_string(ld_json_status2str(status_update->status)));

    return su;
}

json_t *ld_json_unpack_identify_connection_properties(
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

json_t *ld_json_unpack_identify(struct ld_json_identify *identify) {
    json_t *ident = NULL;
//    json_error_t error;
    //todo: add error checking

    ident = json_object();
    if(identify->token == NULL) {
        ld_error("ld_json_unpack_identify: token is NULL");
        return NULL;
    }
    json_object_set(ident, "token", json_string(identify->token));

    if(identify->properties == NULL) {
        ld_error("ld_json_unpack_identify: identify connection properties is NULL");
        return NULL;
    }
    json_object_set(ident, "properties", ld_json_unpack_identify_connection_properties(identify->properties));

    json_object_set(ident, "compress", json_boolean(identify->compress));

    if((identify->large_threshold > 250) || (identify->large_threshold < 50)) {
        ld_warning("ld_json_unpack_identify: large_theshold is out of expected range");
    }
    json_object_set(ident, "large_threshold", json_integer(identify->large_threshold));
    if(&(identify->shard) != NULL) {
        json_t *array = json_array();
        json_array_append_new(array, json_integer(identify->shard[0]));
        json_array_append_new(array, json_integer(identify->shard[1]));
        json_object_set(ident, "shard", array);
    } else {
        ld_error("ld_json_unpack_identify: shard array is NULL");
        return NULL;
    }


    if(identify->status_update != NULL) {
        //presence is optional
        json_object_set(ident, "presence", ld_json_unpack_status_update(identify->status_update));
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
        case LD_PRESENCE_INVISIBLE:
            return "invisible";
            break;
    }
    return NULL;
}

int ld_json_message_init(struct ld_json_message *message) {
    message->id = 0;
    message->channel_id = 0;
    message->author = NULL;
    message->content = NULL;
    message->timestamp = NULL;
    message->edited_timestamp = NULL;
    message->tts = 0;
    message->mention_everyone = 0;
    message->mentions = NULL;
    message->mention_roles = NULL;
    message->attachments = NULL;
    message->embeds = NULL;
    message->reactions = NULL;
    message->webhook_id = 0;
    message->type = 0;
    message->activity = NULL;
    message->application = NULL;
    return 0;
}

int ld_json_message_cleanup(struct ld_json_message *message) {
    if(message->content != NULL) {
        free(message->content);
    }
    if(message->author != NULL) {
        free(message->author);
    }
    return 0;
}

int *ld_json_pack_message(struct ld_json_message *new_message, json_t *message) {
//    struct ld_json_message *new_message = NULL;
//    json_error_t error;
//    message = json_object();
//    int i;
//    void *ret;

    json_t *value;
    const char *key;
    const char *tmp;

    json_object_foreach(message, key, value) {
        if(strcmp(key, "content") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_warning("ld_json_pack_message: couldn't get message content");
            } else {
                new_message->content = strdup(tmp);
            }

        }

        if(strcmp(key, "channel_id") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_warning("ld_json_pack_message: couldn't get message channel_id");
            } else {
                new_message->channel_id = strtoull(tmp, NULL, 10);
            }

        }

        if(strcmp(key, "id") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_warning("ld_json_pack_message: couldn't get message id");
            } else {
                new_message->id = strtoull(tmp, NULL, 10);
            }

        }

        if(strcmp(key, "type") == 0) {
            tmp = json_string_value(value);
            if (tmp == NULL) {
                ld_warning("ld_json_pack_message: couldn't get message type");
            } else {
                new_message->type = (int) strtol(tmp, NULL, 10);
            }

        }
        if(strcmp(key, "author") == 0) {
//            tmp = json_string_value(value);
            new_message->author = malloc(sizeof(struct ld_json_user));
            if(new_message->author == NULL) {
                ld_warning("ld_json_pack_message: couldn't malloc user object");
            } else if(ld_json_load_user(new_message->author, value) != 0) {
                ld_warning("ld_json_pack_message: couldn't read author object properly");
            }
        }

    }

    return 0;

}

//snowflake conversion function
int ld_json_pack_snowflake(struct ld_json_snowflake *new_flake, uint64_t snowflake) {
    if(new_flake == NULL) {
        return LDS_OK;
    }

    uint64_t tmp;

    tmp = (snowflake >> 22);
    new_flake->timestamp = tmp; //discord epoch in ms

    tmp = ((snowflake >> 22) + 1420070400000); //convert to unix time in milliseconds
    new_flake->unix_timestamp = tmp;

    tmp = (snowflake & 0x3E0000) >> 17;
    new_flake->worker_id = tmp;

    tmp = (snowflake & 0x1F000) >> 12;
    new_flake->process_id = tmp;

    tmp = snowflake & 0xFFF;
    new_flake->increment = tmp;

    return LDS_OK;
}

int ld_json_read_timestamp(struct ld_timestamp *new_timestamp, char *timestamp) {
    if(new_timestamp == NULL) {
        return LDS_OK;
    }

    struct tm *time;
    time = getdate(timestamp);

    new_timestamp->iso_str = strdup(timestamp);
    new_timestamp->unix_epoch = (uint64_t) mktime(time);
    new_timestamp->discord_epoch = new_timestamp->unix_epoch * 1000 - 1420070400000;

    return LDS_OK;
}


ld_status _ld_json_dump_all(char **out, json_t *in, const char *caller) {
    char *tmp;
    if(out == NULL) {
        if(caller != NULL)
            ld_warning("%s: recieved null out from %s", __FUNCTION__, caller);
        return LDS_INCOMPLETE_ARGS_ERR;
    }

    tmp = json_dumps(in, 0);
    if(tmp == NULL) {
        *out = NULL;
        ld_warning("%s: jansson: couldn't dump json_t from %s", __FUNCTION__, caller);
        return LDS_JSON_DUMP_ERR;
    }
    *out = tmp;
    return LDS_OK;
}

ld_status ld_json_unpack_resume(json_t *out, struct ld_json_resume *resume) {

    return LDS_OK;
}

ld_status ld_json_dump_resume(char **out, json_t *resume) {
    return _ld_json_dump_all(out, resume, __FUNCTION__);
}

ld_status ld_json_save_resume(char **out, struct ld_json_resume *resume) {
    if(resume == NULL) { //nothing to do
        return LDS_OK;
    }
    if(out == NULL) {
        ld_warning("%s: got null out ptr", __FUNCTION__);
        return LDS_INCOMPLETE_ARGS_ERR;
    }
    if(resume->token == NULL || resume->session_id == NULL) {
        ld_warning("%s: missing token or session id", __FUNCTION__);
        return LDS_JSON_MISSING_REQUIRED_ERR;
    }

    enum ld_status_enum ret;
    json_t json_res;
    ret = ld_json_unpack_resume(&json_res, resume);
    if(ret != LDS_OK) {
        ld_warning("%s: error unpacking");
        return ret;
    }

    ret = ld_json_dump_resume(out, &json_res);
    if(ret != LDS_OK) {
        ld_warning("%s: error dumping", __FUNCTION__);
        return ret;
    }


    return LDS_OK;
}

ld_status ld_json_payload_valid(struct ld_json_websocket_payload *payload) {
    if(payload->op == LD_GATEWAY_OPCODE_UNKNOWN) {
        //bad or unknown opcode
        ld_error("%s: unknown opcode (%d)", __FUNCTION__, payload->op);
        return LDS_JSON_MISSING_REQUIRED_ERR;
    }
    if(payload->d == NULL) {
        //no data payload
        ld_error("%s: no data payload found", __FUNCTION__);
        return LDS_JSON_MISSING_REQUIRED_ERR;
    }

    if(payload->op == LD_GATEWAY_OPCODE_DISPATCH) {
        if(payload->t == LD_DISPATCH_UNKNOWN) {
            ld_error("%s: unknown dispatch type", __FUNCTION__);
            return LDS_JSON_MISSING_REQUIRED_ERR;
        }
        if(payload->s < 0) {
            ld_warning("%s: seq number (%d) is likely invalid (< 0)", __FUNCTION__, payload->s);
        }
    }
    return LDS_OK;
}

ld_status ld_json_unpack_payload(json_t *out, struct ld_json_websocket_payload *payload) {
    if(payload == NULL) { //nothing to do
        return LDS_OK;
    }
    if(ld_json_payload_valid(payload) != LDS_OK) {
        ld_warning("%s: payload struct not valid", __FUNCTION__);
        return LDS_JSON_INVALID_ERR;
    }



    return LDS_OK;
}
ld_status ld_json_dump_payload(char **out, json_t *payload) {
    return _ld_json_dump_all(out, payload, __FUNCTION__);
}
ld_status ld_json_save_payload(char **out, const struct ld_json_websocket_payload *payload) {
    if(out == NULL) {

    }
    return LDS_OK;
}
