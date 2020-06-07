//
// Created by dxing97 on 5/30/20.
//

#ifndef LIBDISCORD_LIBDISCORD_H
#define LIBDISCORD_LIBDISCORD_H

#include "codes.h"
#include "log.h"
#include "libdiscord_config.h"
#include "context.h"


#include <stdio.h>
#include <libwebsockets.h>

/**
 * Initializes per-process global library structures
 * @return Error code
 */
lde_code ld_init(int loglevel);

/**
 *
 * @param context Pointer to initialized ld_context
 * @param timeout Timeout in milliseconds
 * @return
 */
lde_code ld_service(struct ld_context *context, int timeout);

#endif //LIBDISCORD_LIBDISCORD_H
