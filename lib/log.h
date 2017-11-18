//
// Created by danielxing.6 on 11/17/2017.
//

#ifndef LIBDISCORD_LOG_H
#define LIBDISCORD_LOG_H

#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "libdiscord.h"

/*
 * logging levels are bitfields set in a ld context
 *  error:      something bad and irrecoverable has happened
 *  warning:    something bad has happened and I will try to recover
 *  notice:     something has happened that requires user attention
 *  info:       something happened
 *  debug:      debug information
 */
enum ld_log_level {
    ld_log_error = 1<<0,
    ld_log_warning = 1<<1,
    ld_log_info = 1<<2,
    ld_log_notice = 1<<3,
    ld_log_debug = 1<<4
};

/*
 * private logging function
 * level: bit-shifted logging level
 * enabled levels: bit pattern of all enabled levels
 * arg: va_list
 *
 * log format:
 * [DATE TIME] libdiscord [logging type]: ARGS
 */
void _ld_log(unsigned long ll, unsigned long enabled_levels, const char *log_message, va_list arg);

const char *ld_log_level_string(unsigned long ll);

#endif //LIBDISCORD_LOG_H
