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
 *
 *      will only check latencies when requested to do so by a user
 *          this means it won't check the latency for intial websocket connections (for now)
 */
 
#include <libdiscord.h>
#include <getopt.h>
#include <signal.h>

static int bot_exit = 0;

void int_handler(int i){
    bot_exit = 1;
}

int callback(struct ld_context *context, enum ld_callback_reason reason, void *data, int len);

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
                printf("libdiscord example bot: ping - comprehensive latency measurement tool\n"
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
            default:
                abort();
        }
    }

    printf("Example bot 2 \"ping\" starting up using libdiscord v%s\n", LD_VERSION);

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

    info->bot_token = bot_token;
    info->user_callback = callback;
    info->gateway_ringbuffer_size = 8;

    struct ld_context *context = malloc(sizeof(struct ld_context));
    if(ld_init_context(info, context) != LDS_OK) {
        ld_error("error creating libdiscord context");
        return 1;
    }
    free(bot_token);
    free(info);
    
    // create context info struct
    // init context
    // start loop
    int bot_state = 0, ret;
    while(!bot_exit) {
        if(bot_state == 0) {
            ret = ld_connect(context);
            if(ret != 0) {
                ld_warning("error connecting to discord: error code %d");
                break;
            }
            bot_state = 1;
        }
        ret = ld_service(context, 20);
        if(ret != 0) {
            ld_error("ld_service returned nonzero {%d}", ret);
            break;
        }
    }

    ld_info("closing connections and exiting");
    ld_cleanup_context(context);
    
    return 0;
}

int callback(struct ld_context *context, enum ld_callback_reason reason, void *data, int len) {
    
    return 0;
}