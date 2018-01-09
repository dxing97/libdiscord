//
// Created by danielxing.6 on 11/17/2017.
//

#include "log.h"

const char *ld_log_level_string(unsigned long ll) {
    switch(ll) {
        case ld_log_error:
            return "ERROR";
        case ld_log_warning:
            return "WARNING";
        case ld_log_info:
            return "INFO";
        case ld_log_notice:
            return "NOTICE";
        case ld_log_debug:
            return "DEBUG";
        default:
            //sum ting wong
            return "UNKNOWN(?)";
    }
}

void _ld_log(unsigned long ll, unsigned long enabled_levels, const char *log_message, va_list arg){
    if((ll & enabled_levels) == 0)
        return;
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
        perror("couldn't create log message");
        return;
    }

    message = malloc(strlen(time_string) + strlen(ld_log_level_string(ll)) + strlen(msg) + 15);
    sprintf(message, "[%s] LD_%s: %s", time_string, ld_log_level_string(ll), msg);
    fprintf(stderr, "%s\n", message);

    free(time_string);
    free(message);
}
