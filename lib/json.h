//
// Created by dxing97 on 1/15/18.
//

#ifndef LIBDISCORD_JSON_H
#define LIBDISCORD_JSON_H

#include <jansson.h>

/*
 * takes four json_t objects and creates a payload
 */
json_t *ld_json_create_payload(json_t *op, json_t *d, json_t *t, json_t *s);

#endif //LIBDISCORD_JSON_H
