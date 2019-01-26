//
// Created by dxing97 on 11/10/18.
//

#include <getopt.h>

#include <libdiscord.h>

void print_help(char *executable_name) {
    printf("libdiscord message posting test bot\n"
           "%s [-t bot_token] [-m message] [-c channel_id]\n\n"
           "Options: \n\t"
           "-t, --bot-token [bot_token]\n\t\t"
           "Required. Discord bot token. See Discord developer pages on how to obtain one.\n\t"
           "-m, --message \"[message]\"\n\t\t"
           "Required. Message to be posted\n\t"
           "-c, --channel [channel_id]\n\t\t"
           "Required. Discord channel ID to post in. Bot requires permissions for posting new messages in that channel.\n\t"
           "-h, --help\n\t\t"
           "Displays this help dialog\n", executable_name);
}

int main(int argc, char *argv[]) {

    char *bot_token = NULL;
    char *message = NULL;
    LD_SNOWFLAKE channel = 0;
    unsigned long log_level = 63;

    if(argc == 1) {
        goto HELP;
    }

    while(1) {
        int option_index = 0, c;
        static struct option long_options[] = {
                {"help",      no_argument,       0, 'h'},
                {"bot-token", required_argument, 0, 't'},
                {"log-level", required_argument, 0, 'l'},
                {"message", required_argument, 0, 'm'},
                {"channel", required_argument, 0, 'c'},
                {0, 0,                           0, 0}
        };
        c = getopt_long(argc, argv, "ht:l:m:c:", long_options, &option_index);
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
            case 'm':
                message = strdup(optarg);
                break;
            case 'c':
                channel = strtoull(optarg, NULL, 10);
                break;
            default:
                return 1; //this should never happen
        }
    }

    if(message == NULL) {
        ld_notice("missing message content");
        return 0;
    }

    if(channel == 0) {
        ld_notice("missing channel id");
        return 0;
    }

    if(bot_token == NULL) {
        ld_notice("missing bot token");
        return 0;
    }

    int retv;
    struct ld_context_info info;
    retv = ld_init_context_info(&info);
    if(retv != 0) {
        return 2;
    }

    info.bot_token = bot_token;

    struct ld_context context;
    retv = ld_init_context(&info, &context);
    if(retv != 0) {
        return 3;
    }

    retv = ld_send_basic_message(&context, channel, message);
    if(retv != 0) {
        return 4;
    }
    return 0;
}
