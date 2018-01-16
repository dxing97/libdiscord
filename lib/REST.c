//
// Created by dxing97 on 1/15/18.
//

#include "REST.h"

struct ld_rest_request *ld_rest_init_request() {
    struct ld_rest_request *req;
    req = malloc(sizeof(struct ld_rest_request));
    if(req == NULL) {
        ld_error("couldn't allocate memory for restrequest");
        return NULL;
    }
    req->base_url = NULL;
    return req;
}
