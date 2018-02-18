//
// Created by danielxing.6 on 11/21/2017.
//

/*
 * ping bot example
 *
 * potential response times to measure:
 *
 *      latency between sending a HTTP request and receiving a response for endpoints such as:
 *
 *          GET /gateway
 *          DELETE [message]
 *          POST [message]
 *          GET [messages]
 *
 *          for both blocking (ulfius) and nonblocking (libcurl) methods
 *
 *
 *      latency between a HTTP request and it's corresponding gateway dispatch, such as:
 *          POST [message] -> G RX DISPATCH new [message]
 *
 *      latency between a sent gateway payload and a response from the gateway
 *          G TX STATUS_UPDATE -> G RX DISPATCH new [status]
 *          G TX HEARTBEAT -> G RX HEARTBEAT_ACK
 */
#include <libdiscord.h>
#include <getopt.h>

int bot_exit = 0;

void int_handler(int i){
    bot_exit = 1;
}

int main(int argc, char *argv[]) {
    ld_set_logging_level(31);

    int c; //bot_exit: 0 for don't exit, 1 for exit
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
        c = getopt_long(argc, argv, "t:hl:", long_options, &option_index);

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
                               "Required. Discord bot token. See Discord developer pages on how to obtain one.\n\t"
                               "-l, --log-level [level] \n\t\t"
                               "Enables certain logging levels. Default is 31 (everything)."
                               "-h, --help\n\t\t"
                               "Displays this help dialog\n", argv[0]);
                return 0;
            case 't':
                bot_token = strdup(optarg);
                break;
            case 'l':
                log_level = strtoul(optarg, NULL, 10);
                break;
                break;
            default:
                abort();
        }
    }

    printf("Example bot 2 \pinger\" starting up using libdiscord v%s\n", LD_VERSION);

    if(bot_token == NULL){
        printf("Bot token not set! See example-ayylmao -h for details.");
        return 1;
    }
    printf("Initializing libdiscord with log level %lu\n", log_level);

    ld_set_logging_level(log_level);

    signal(SIGINT, int_handler);

    struct ld_context_info *info;
    info = malloc(sizeof(struct ld_context_info));
    if(info == NULL) {
        ld_error("couldn't allocate context info");
        return 1;
    }


    return 0;
}