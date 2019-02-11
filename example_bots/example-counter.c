//
// Created by dxing97 on 11/10/18.
//

#include <libdiscord.h>
#include <getopt.h>
#include <signal.h>
/*
 * counting bot
 */

int bot_exit = 0;
float response_chance = 0.01;

int callback(struct ld_context *context, enum ld_callback_reason reason, void *data, int len) {
    if (reason == LD_CALLBACK_MESSAGE_CREATE) {
        struct ld_json_message message;
        ld_json_message_init(&message);

        ld_json_pack_message(&message, (json_t *) data);

        char *next;
        if (message.content == NULL) {
            ld_debug("empty message!");
            return 0;
        }

        if (strcmp(message.content, "hcf") == 0) {
            //halt
            bot_exit = 1;
        }

        long long count = strtoll(message.content, &next, 10);
        if (next == message.content) {
            ld_debug("not a number");
            return 0;
        }



//        if(message.channel_id != target_channel){
//            //not in target channel
//            ld_debug("not in target channel");
//            return 0;
//        }

        if (message.author->id == context->current_user->id) {
            ld_debug("can't respond to ourselves");
            return 0;
        }

        if (message.author->bot == 1) {
            ld_debug("shouldn't respond to another bot");
            return 0;
        }

        if ((float) rand() / (float) RAND_MAX > response_chance) { // 1% chance of responding
            return 0;
        }

        char new_message[64];
        sprintf(new_message, "%lld", count + 1);
        ld_send_basic_message(context, message.channel_id, new_message);

        return 0;
    }


    return 0;
}

void sig_handler(int i) {
    bot_exit = 1;
}

void print_help(char *executable_name) {
    printf("libdiscord counter bot\n"
           "%s [-t bot_token]\n\n"
           "Options: \n\t"
           "-t, --bot-token [bot_token]\n\t\t"
           "Required. Discord bot token. See Discord developer pages on how to obtain one.\n\t"
           "-h, --help\n\t\t"
           "Displays this help dialog\n", executable_name);
}


int main(int argc, char *argv[]) {
    char *bot_token = NULL;
    unsigned long log_level = 63;

    if (argc == 1) {
        goto HELP;
    }

    while (1) {
        int option_index = 0, c;
        static struct option long_options[] = {
                {"help",      no_argument,       0, 'h'},
                {"bot-token", required_argument, 0, 't'},
                {"log-level", required_argument, 0, 'l'},
                {0, 0,                           0, 0}
        };
        c = getopt_long(argc, argv, "ht:l:", long_options, &option_index);
        if (c == -1) {
            break;
        }
        switch (c) {
            case 'h':
            HELP:
                print_help(argv[0]);
                return 0;
            case 't':
                if (strlen(optarg) > 65) { //basic overflow check
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

    if (bot_token == NULL) {
        printf("bot token is not set! see %s -h for details on how to pass your bot token in\n", argv[0]);
        return 0;
    }

    signal(SIGINT, sig_handler);

    printf("Initializing libdiscord at log level %lu\n", log_level);

    ld_set_logging_level(log_level);

    struct ld_context_info info;
    if (ld_init_context_info(&info) != 0) {
        ld_error("%s: couldn't init context info", __FUNCTION__);
    }
    info.bot_token = bot_token;
    info.user_callback = callback;

    srand((unsigned int) time(NULL));
    struct ld_context *context = malloc(sizeof(struct ld_context));
    int ret = ld_init_context(&info, context);
    if (ret != 0) {
        ld_error("example-minimal: couldn't initalize context");
        goto HALT;
    }
    if(context->curl_handle == NULL) {
        ld_error("curl handle is null");
        return 1;
    }
    int bot_state = 0; //not connected initially
    while (!bot_exit) {
        if (bot_state == 0) {
            ret = ld_connect(context);
            if (ret != 0) {
                ld_error("example-minimal: error connecting to discord (error %d)", ret);
                bot_exit = 1;
            }
            bot_state = 1;
        } else {
            ret = ld_service(context, 20);
            if (ret != 0) {
                ld_error("example-minimal: ld_service returned error (error %d)", ret);
                bot_exit = 1;
            }
        }
    }

    //ld_send_basic_message(&context, 345264084679131146, "example-bot-counter: received SIGINT or HCF, cleaning up");

    HALT:
    //ld_cleanup_context(&context);
    free(bot_token);
    return 0;
}