//
// Created by dxing97 on 5/30/20.
//

#include "libdiscord.h"

#include <time.h>

lde_code ld_init(int loglevel) {
    lws_set_log_level(loglevel, NULL);
    lwsl_info("libdiscord init with loglevel %d", loglevel);

    return LDE_OK;
}

lde_code ld_service(struct ld_context *context, int timeout) {
    usleep(timeout * 1000);
    return LDE_OK;
}