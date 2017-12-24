//
// Created by dxing97 on 11/11/17.
//

#include "libdiscord.h"

/*
 * This bot will respond to every "ayy" with "lmao"
 */

#include <getopt.h>

/*
 * main way of user interaction with libdiscord
 * the user callback returns 0 if everything is OK
 *
 * ld_context is an opaque internal bot identifier.
 * ld_callback_reason is the reason for the library calling the callback.
 * data and len contain data that may be needed for the callback, their use depends on the reason for the callback.
 *
 * if the return value is not 0, libdiscord will interpret it as that to mean disconnect from Discord.
 */
int callback(struct ld_context *context, enum ld_callback_reason reason, const char *data, int len) {
    /*
     * depending on the reason, do stuff
     */
    //if the reason is a gateway event, and the payload is a dispatch, and the dispatch contains a message JSON, and the message has content "ayy" (case statement?)
        //post a "lmao" to the channel
        //create a new message info struct with content "lmao"
        //add that context to the send queue
    switch(reason){
        case LD_WEBSOCKET_RECEIVE:
            ld_debug(context, "Recieved data from gateway.");
            break;
        case LD_WEBSOCKET_SENDABLE:
            ld_debug(context, "We can now send data to the gateway");
            break;
        case LD_WEBSOCKET_CONNECTING:
            ld_debug(context, "We are now connecting to the gateway");
            break;
    }
    return 0;
}

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
    int c, bot_exit = 0; //bot_exit: 0 for don't exit, 1 for exit
    char *bot_token = NULL;
    unsigned long log_level = 31;
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
    info->user_callback = (&callback);
    free(bot_token);

    //initialize context with context info
    struct ld_context *context;
    context = ld_create_context_via_info(info); //garbage in, garbage out
    if(context == NULL) {
        fprintf(stderr, "error creating ld context\n");
        return 1;
    }
    free(info);

    int ret;

    //while the bot is still alive
    while(!bot_exit) {
        //if the bot isn't connected to discord, connect to discord
        if((ld_gateway_connection_state(context) != LD_GATEWAY_CONNECTED) &&
                (ld_gateway_connection_state(context) != LD_GATEWAY_CONNECTING)) {
            //currently unconnected, will now connect
            ld_info(context, "Connecting to Discord...");
            ret = ld_connect(context);
            if(ret != 0) {
                ld_warn(context, "couldn't connect to Discord: error code %d", ret);
            }
            bot_exit = 1;
        } else  if(ld_gateway_connection_state(context) == LD_GATEWAY_DISCONNECTED){
            //we've been disconnected for some reason, so we should figure out why we were disconnected
            ld_info(context, "Checking disconnection reason...");
            bot_exit = 1;
        }

        //what if we're not disconnected/unconnected, but just simply still in the process of connecting?
        //what does it mean to be "connecting"?
        //service the connection
        ld_debug(context, "servicing connection");
    }
    //disconnect from discord gracefully
    ld_info(context, "disconnected from discord");
    //destroy the context
    ld_destroy_context(context);
    return 0;
}
