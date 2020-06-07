//
// Created by dxing97 on 5/30/20.
//

#include "context.h"
#include "REST.h"
#include "log.h"

#include <string.h>
#include <stdlib.h>

lde_code ld_context_init(struct ld_context_options *opts, struct ld_context *context) {
    context->token = strdup(opts->bot_token);
    if(ld_rest_init() != LDE_OK) {
        lwsl_err("%s: %s: %d: couldn't initialize libcurl", __FILE__, __FUNCTION__, __LINE__);
    }
    return LDE_OK;
}

lde_code ld_context_destroy(struct ld_context *context) {
    free(context->token);
    ld_rest_cleanup();
    return LDE_OK;
}