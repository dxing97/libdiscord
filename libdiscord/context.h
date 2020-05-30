//
// Created by dxing97 on 5/30/20.
//

#ifndef LIBDISCORD_CONTEXT_H
#define LIBDISCORD_CONTEXT_H

#include "codes.h"

enum ld_callback_reason {
    NONE = 0
};

struct ld_context_options;
struct ld_context;

struct ld_context_options {
    char *bot_token; /// allocated and freed by client
    int (*user_callback)
            (struct ld_context *context, enum ld_callback_reason reason, void *data, int len);
};

struct ld_context {
    int *token;
};

/**
 * Initializes per-process global library structures
 * @return Error code
 */
lde_code ld_init(int loglevel);

/**
 * Initializes libdiscord context given options
 * @return
 */
lde_code ld_context_init(struct ld_context_options *opts, struct ld_context *context);

lde_code ld_context_destroy(struct ld_context *context);
#endif //LIBDISCORD_CONTEXT_H
