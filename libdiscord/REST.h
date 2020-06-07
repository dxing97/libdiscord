//
// Created by dxing97 on 6/6/20.
//

#ifndef LIBDISCORD_REST_H
#define LIBDISCORD_REST_H

#include "codes.h"

#include <jansson.h>


lde_code ld_rest_init();
lde_code ld_rest_cleanup();

lde_code ld_rest_get_gateway(json_t *response);


#endif //LIBDISCORD_REST_H
