//
// Created by dxing97 on 11/10/18.
//

#include <libdiscord.h>
#include <getopt.h>

/*
 * minimal bot
 */

int bot_exit = 0;

int callback(struct ld_context *context, enum ld_callback_reason reason, void *data, int len) {

    return 0;
}

void print_help(char *executable_name) {
printf("libdiscord minimal bot\n"
       "%s [-t bot_token]\n\n"
       "Options: \n\t"
       "-t, --bot-token [bot_token]\n\t\t"
       "Required. Discord bot token. See Discord developer pages on how to obtain one.\n\t"
       "-h, --help\n\t\t"
       "Displays this help dialog\n", executable_name);
}


int main(int argc, char *argv[]) {
    char *bot_token = NULL;
    unsigned long log_level = 31;

    if(argc == 1) {
        goto HELP;
    }

    while(1) {
        int option_index = 0, c;
        static struct option long_options[] = {
                {"help",      no_argument,       0, 'h'},
                {"bot-token", required_argument, 0, 't'},
                {"log-level", required_argument, 0, 'l'},
                {0,0,0,0}
        };
        c = getopt_long(argc, argv, "ht:l:", long_options, &option_index);
        if(c == -1) {
            break;
        }
        switch(c) {
            case 'h':
            HELP:
                print_help(argv[0]);
                return 0;
            case 't':
                if(strlen(optarg) > 65) { //basic overflow check
                    printf("invalid bot token detected");
                    return 0;
                }
                bot_token = strdup(optarg);
                break;
            case 'l':
                log_level = strtoul(optarg, NULL, 10);
                break;
            default:
                return 1; //this should never happen
        }
    }

    printf("Example bot 3 \"minimal\" starting up using libdiscord v%s\n", LD_VERSION);

    if(bot_token == NULL) {
        printf("bot token is not set! see %s -h for details on how to pass your bot token in\n", argv[0]);
        return 0;
    }

    printf("Initializing libdiscord at log level %lu\n", log_level);

    ld_set_logging_level(log_level);
    //switch over to libdiscord logging functions

    struct ld_context_info info;

    info.init_presence = NULL;
    info.bot_token = bot_token;
    info.user_callback = callback;
    info.gateway_ringbuffer_size = 8;

    void *ret;
    struct ld_context context;
    ret = ld_init_context(&context, &info);
    if(ret == NULL) {
        ld_error("example-minimal: couldn't initalize context");
    }
    int bot_state = 0; //not connected initially
    while(!bot_exit) {
        if(bot_state == 0) {
            ret = ld_connect(&context);
            if(ret != 0) {
                ld_error("example-minimal: error connecting to discord (error %d)", ret);
                bot_exit = 1;
            }
        }
        ret = ld_service(&context, 20);
        if(ret != 0) {
            ld_error("example-minimal: ld_service returned error (error %d)", ret);
            bot_exit = 1;
        }
    }

    ld_destroy_context(&context);
    free(bot_token);
    return 0;
}