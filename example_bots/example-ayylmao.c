//
// Created by dxing97 on 11/11/17.
//

#include "libdiscord.h"

/*
 * This bot will respond to every "ayy" with "lmao"
 */

#include <getopt.h>

//int callback() {
    /*
     * depending on the reason, do stuff
     */
    //if the reason is a gateway event, and the payload is a dispatch, and the dispatch contains a message JSON, and the message has content "ayy" (case statement?)
        //post a "lmao" to the channel
        //create a new message info struct with content "lmao"
        //add that context to the send queue
//}

int main(int argc, char *argv[]) {

    /*
     * pass in arguments for parameters that we want to connect to discord with.
     *  this includes the bot token, debug verbosity, and so on
     *  use getopt
     * initialize a connection to the gateway
     *
     * listen for messages sent with "ayy" (with permutations)
     * if an "ayy" is detected, POST a message to the same channel with content "lmao"
     * continue ad infinitum (or until the bot is killed)
     */
    int c;
    char *bot_token = NULL;
    unsigned long log_level = 15;
    if(argc == 1) {
        goto HELP;
    }

    while(1) {
        //options: help, bot token
        //if bot token isn't specified, exit
        static struct option long_options[] = {
                {"bot-token", required_argument, 0, 't'},
                {"help", no_argument, 0, 'h'},
                {"log-level", required_argument, 0, 'l'},
                {0,0,0,0}
        };
        int option_index = 0;
        c = getopt_long(argc, argv, "ht:l:", long_options, &option_index);
        if(c == -1){
            break;
        }

        switch(c) {
            case 'h':
            HELP:
                printf("libdiscord example bot: ayylmao\n"
                               "%s [-t bot_token]\n\n"
                               "Options: \n\t"
                               "-t, --bot-token [bot_token]\n\t\t"
                               "Discord bot token. See Discord developer pages on hwo to obtain one\n\t"
                               "-h, --help\n\t\t"
                               "Displays this help dialog\n", argv[0]);
                return 0;
            case 't':
                bot_token = strdup(optarg);
                break;
            case 'l':
                log_level = strtoul(optarg, NULL, 10);
                break;
            default:
                abort();
        }
    }

    printf("Example bot 1 \"ayylmao\" starting up using libdiscord v%s\n", LD_VERSION);

    printf("Bot token: %s\n", bot_token);
    printf("Initializing libdiscord with log level %lu\n", log_level);

    //define context info, including bot token
    struct ld_context_info *info;
    info = malloc(sizeof(struct ld_context_info));

    info->bot_token = strdup(bot_token);
    info->log_level = log_level;

    free(bot_token);

    //initialize context with context info
    struct ld_context *context;
    context = ld_create_context_via_info(info);
    if(context == NULL) {
        fprintf(stderr, "error creating ld context\n");
        return -1;
    }
    free(info);

    int exit = 0;
    //while the bot is still alive
    while(!exit) {
        //if the bot isn't connected to discord, connect to discord
        if(ld_connection_state(context) == 0) {
            ld_info(context, "Connecting to Discord...");
            ld_connect(context);
        }
        //service the connection
        ld_debug(context, "servicing connection");
        exit = 1;
    }
    //disconnect from discord gracefully
    ld_info(context, "disconnected from discord");
    //destroy the context
    ld_destroy_context(context);
    return 0;
}
