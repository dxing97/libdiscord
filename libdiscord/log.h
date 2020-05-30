//
// Created by dxing97 on 5/30/20.
//

#ifndef LIBDISCORD_LOG_H
#define LIBDISCORD_LOG_H

#include <libwebsockets.h>

enum LD_LL {
    LD_ERR = LLL_ERR,
    LD_WARN = LLL_WARN,
    LD_NOTICE = LLL_NOTICE,
    LD_INFO = LLL_INFO,
    LD_DEBUG = LLL_DEBUG
};

#endif //LIBDISCORD_LOG_H
