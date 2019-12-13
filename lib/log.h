/** @file */ 

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
#include "status.h"

/*
 * logging levels are bitfields set in a ld context
 *  error:      something bad and irrecoverable has happened
 *  warning:    something bad has happened and I will try to recover
 *  notice:     something has happened that requires user attention
 *  info:       something happened
 *  debug:      debug information
 */
enum ld_log_level {
    LD_LOG_ERROR = 1<<0,
    LD_LOG_WARNING = 1<<1,
    LD_LOG_NOTICE = 1<<2,
    LD_LOG_INFO = 1<<3,
    LD_LOG_DEBUG = 1<<4
};

/*
 * sets the static global variable for logging function
 * eliminates the need for keeping the log level in ld_context
 * can call whenever, sets a static global variable in library
 */
unsigned long ld_set_logging_level(unsigned long log_level);

/*
 * logging functions
 * can use variable length arguments like *printf functions
 * example:
   ld_notice("shard: %d", shard_number); //where shard_number is an int
   replaces old logging functions requiring ld_context as argument
 */
void ld_error(const char *message, ...);
void ld_warning(const char *message, ...);
void ld_info(const char *message, ...);
void ld_notice(const char *message, ...);
void ld_debug(const char *message, ...);

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

/*
 * returns a read-only string for the specified log level
 */
const char *ld_log_level_string(unsigned long ll);

/*
 * returns current log level
 */
unsigned long ld_get_logging_level();

/*
 * allocates and returns a string containing a log message that can be sent to stderr or a discord channel
 */
char *ld_log_generate_string(unsigned long ll, const char *log_message, va_list arg);

#endif //LIBDISCORD_LOG_H
