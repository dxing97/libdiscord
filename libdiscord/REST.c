//
// Created by dxing97 on 6/6/20.
//

#include "REST.h"
#include <curl/curl.h>

lde_code ld_rest_init() {
    curl_global_init(CURL_GLOBAL_ALL);
    return LDE_OK;
}

lde_code ld_rest_cleanup() {
    curl_global_cleanup();
    return LDE_OK;
}

lde_code ld_rest_get_gateway(json_t *response) {


    return LDE_OK;
}