//
// Created by danielxing.6 on 11/17/2017.
//

#include "log.h"

/*
 * internal macro used for logging
 */
#define __LD_LOG(logtype) \
    if((_ld_ll & (logtype)) == 0) {\
        return;\
    }\
    va_list myargs;\
    va_start(myargs, message);\
    _ld_log(logtype, _ld_ll, message, myargs)

/*
 * private global variable used to store the current logging level.
 */
static unsigned long _ld_ll = LD_LOG_ERROR | LD_LOG_WARNING | LD_LOG_NOTICE | LD_LOG_INFO;

//sets static global variable for the library
unsigned long ld_set_logging_level(unsigned long log_level) {
    _ld_ll = log_level;
    return _ld_ll;
}

/*
 * logging functions
 */
void ld_error(const char *message, ...) {
    __LD_LOG(LD_LOG_ERROR);
}

void ld_warning(const char *message, ...) {
    __LD_LOG(LD_LOG_WARNING);
}

void ld_info(const char *message, ...) {
    __LD_LOG(LD_LOG_INFO);
}

void ld_notice(const char *message, ...) {
    __LD_LOG(LD_LOG_NOTICE);
}

void ld_debug(const char *message, ...) {
    __LD_LOG(LD_LOG_DEBUG);
}

//private function used by library
const char *ld_log_level_string(unsigned long ll) {
    switch(ll) {
        case LD_LOG_ERROR:
            return "ERROR";
        case LD_LOG_WARNING:
            return "WARNING";
        case LD_LOG_INFO:
            return "INFO";
        case LD_LOG_NOTICE:
            return "NOTICE";
        case LD_LOG_DEBUG:
            return "DEBUG";
        default:
            //sum ting wong
            return "UNKNOWN(?)";
    }
}

//private function used by logging system
void _ld_log(unsigned long ll, unsigned long enabled_levels, const char *log_message, va_list arg){
    if((ll & enabled_levels) == 0)
        return;
//    time_t raw_time;
//    struct tm *current_time;
    char *message;
//    char *time_string;
//    //[2017/08/18 19:19:18]
//
//    //time part
//    time_string = malloc(30 * sizeof(char));
//    time(&raw_time);
//    current_time = localtime(&raw_time);
//    asctime_r(current_time, time_string);
//    time_string[24] = '\0';
//
//    //message part
//    char msg[16000]; //watch for buffer overflows here
//    int ret;
//
//    ret = vsprintf(msg, log_message, arg);
//
//    if(ret < 0){
//        perror("libdiscord: couldn't create log message");
//        return;
//    }
//
//    message = malloc(strlen(time_string) + strlen(ld_log_level_string(ll)) + strlen(msg) + 15);
//    sprintf(message, "[%s] LD_%s: %s", time_string, ld_log_level_string(ll), msg);
//    fprintf(stderr, "%s\n", message);
//
//    free(time_string);
    message = ld_log_generate_string(ll, log_message, arg);
    if(message == NULL) {
        perror("libdiscord counldn't print an error message");
        return;
    }
    fprintf(stderr, "%s\n", message);
    free(message);
}

/*
 * generates a log string
 * string memory is allocated by this function, it must be freed afterward
 * todo: make it read-only (and avoid the free) (how??)
 */
char *ld_log_generate_string(unsigned long ll, const char *log_message, va_list arg) {
    time_t raw_time;
    struct tm *current_time;
    char *message, *time_string;
    //[2017/08/18 19:19:18]

    //time part
    time_string = malloc(30 * sizeof(char));
    time(&raw_time);
    current_time = localtime(&raw_time);
    asctime_r(current_time, time_string);
    time_string[24] = '\0';

    //message part
    char msg[16000]; //watch for buffer overflows here
    int ret;

    ret = vsprintf(msg, log_message, arg);

    if(ret < 0){
        perror("libdiscord: couldn't create log message");
        return NULL;
    }

    message = malloc(strlen(time_string) + strlen(ld_log_level_string(ll)) + strlen(msg) + 15);
    sprintf(message, "[%s] LD_%s: %s", time_string, ld_log_level_string(ll), msg);
    free(time_string);
    return message;
}


/*
 * return log level
 */
unsigned long ld_get_logging_level() {return _ld_ll;};
