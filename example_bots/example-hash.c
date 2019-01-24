//
// Created by dxing97 on 2018-12-14.
//

#include <libdiscord.h>
#include <getopt.h>

#include <openssl/evp.h> //openssl EVP crypto API

/*
 * bot that hashes stuff using a specified hashing function
 * bot arguments (in Discord):
 *  !hash <hash function> <stuff to hash>
 *  i.e.
 *  !hash md5 MY MESSAGE HERE
 */

int bot_exit = 0;

char trigger[] = "!hash";

//unsigned long long target_channel = 342013131121229824;

struct buffer {
    char *buf;
    unsigned int len;
};

char *binary2hex(unsigned char *buf,int  buflen) {
    int i = 0;
    char tmp[3];
    char *hex = malloc(sizeof(char) * (buflen * 2 + 1));
    for(; i < buflen; i++) {
        sprintf(tmp, "%02x", buf[i]);
        strcat(hex, tmp);
    }

//    printf("hex: %s\n", hex);
    return hex;
}

char *to_hash2(const char *input, const char *hashfun) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    const char *mess1 = input;
    char mess2[] = "";
    unsigned char md_value[EVP_MAX_MD_SIZE];
    int md_len, i;


    md = EVP_get_digestbyname(hashfun);

    if(!md) {
        //printf("Unknown message digest %s\n", hashfun);
        return NULL;
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, mess1, strlen(mess1));
    EVP_DigestUpdate(mdctx, mess2, strlen(mess2));
    EVP_DigestFinal_ex(mdctx, md_value, &md_len);
    EVP_MD_CTX_free(mdctx);

    return binary2hex(md_value, md_len);
}

//int hashfun(EVP_MD *md, char *content, struct buffer *buffer) {
//    EVP_MD_CTX *mdctx;
//
//    unsigned char md_value[EVP_MAX_MD_SIZE];
//    unsigned int md_len;
//
//    mdctx = EVP_MD_CTX_create();
//    EVP_DigestInit_ex(mdctx, md, NULL);
//    EVP_DigestUpdate(mdctx, content, strlen(content));
////    EVP_DigestUpdate(mdctx, mess2, strlen(mess2));
//    EVP_DigestFinal_ex(mdctx, md_value, (unsigned int *) &md_len);
//    EVP_MD_CTX_destroy(mdctx);
//
//    /*
//    debug_printfnln("token2key: hashed token is ");
//    for(i = 0; i < md_len; i++)
//        debug_printfnln("%02x", md_value[i]);
//    debug_printf("\n");*/
//
//    buffer->buf = malloc(sizeof(char) * md_len + 10);
//    buffer->len = md_len;
//
//    buffer->buf = memcpy(buffer->buf, md_value, md_len); //we only want first 128 bits
//
//    /* Call this once before exit. */
//    EVP_cleanup();
//
//    return 0;
//}

//takes message and extracts the message we want to hash
char *to_hash(struct ld_json_message *message, struct ld_context *context) {
    char *message_content = message->content;
    if(strlen(message_content) < strlen(trigger) + 4) { //message is too short, doesn't include trigger
        return NULL;
    }
    if(strncmp(message_content, trigger, strlen(trigger)) != 0) { //message doesn't contain trigger
        return NULL;
    }

    //get hash function
    char *head;
    head = message_content + strlen(trigger); //head is at space after trigger
    if(*head != ' ') {
        return NULL;
    }
    head++;

    char hash_function[strlen(message_content)];
    if(sscanf(head, "%s", hash_function) != 1) { //couldn't get hash function
        return NULL;
    }

    char *ret, tmp[100];

//    sprintf(tmp, "hashing (%s)", head + strlen(hash_function) + 1);
//    ld_send_basic_message(context, message->channel_id, tmp);

    ret = to_hash2(head + strlen(hash_function) + 1, hash_function);
    if(ret == NULL) {
        sprintf(tmp, "invalid hash function (%s)", hash_function);
        ld_send_basic_message(context, message->channel_id, tmp);
    }

    return ret;

}

int callback(struct ld_context *context, enum ld_callback_reason reason, void *data, int len) {
    if(reason == LD_CALLBACK_MESSAGE_CREATE) {
        struct ld_json_message message;
        ld_json_message_init(&message);

        ld_json_load_message(&message, (json_t *) data);

        char *next;
        if(message.content == NULL) {
            ld_debug("empty message!");
            return 0;
        }

        if(strcmp(message.content, "hcf") == 0) {
            //halt
            bot_exit = 1;
        }


//        long long count = strtoll(message.content, &next, 10);
//        if(next == message.content) {
//            ld_debug("not a number");
//            return 0;
//        }

//        if(message.channel_id != target_channel){
//            //not in target channel
//            ld_debug("not in target channel");
//            return 0;
//        }

        if(message.author->id == context->current_user->id) {
            ld_debug("can't respond to ourselves");
            return 0;
        }

        if(message.author->bot == 1) {
            ld_debug("shouldn't respond to another bot");
            return 0;
        }

        char *hash = to_hash(&message, context);
        if(hash == NULL) {
            //message wasn't formatted correctly, do nothing
            return 0;
        }

        ld_send_basic_message(context, message.channel_id, hash);
        free(hash);

//        if(rand() % 3 == 0) { //33 % chance of responding
//            return 0;
//        }

//        char channelid[64], new_message[64];
//        sprintf(channelid, "%llu", message.channel_id);
//        sprintf(new_message, "%lld", count + 1);
//        ld_send_basic_message(context, channelid, new_message);

        return 0;
    }


    return 0;
}

void sig_handler(int i) {
    bot_exit = 1;
}

void print_help(char *executable_name) {
    printf("libdiscord hash bot\n"
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

    if(argc == 1) {
        goto HELP;
    }

    while(1) {
        int option_index = 0, c;
        static struct option long_options[] = {
                {"help",      no_argument,       0, 'h'},
                {"bot-token", required_argument, 0, 't'},
                {"log-level", required_argument, 0, 'l'},
                {0, 0,                           0, 0}
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

    printf("Example bot 5 \"hash\" starting up using libdiscord v%s\n", LD_VERSION);

    if(bot_token == NULL) {
        printf("bot token is not set! see %s -h for details on how to pass your bot token in\n", argv[0]);
        return 0;
    }

    signal(SIGINT, sig_handler);

    printf("Initializing libdiscord at log level %lu\n", log_level);

    ld_set_logging_level(log_level);

    struct ld_context_info info;
    if(ld_init_context_info(&info) != 0) {
        ld_error("%s: error initalizing context info");
        goto ABORT;
    }
    info.bot_token = bot_token;
    info.user_callback = callback;

    struct ld_json_status_update presence;
    struct ld_json_activity game;
    presence.status = LD_PRESENCE_ONLINE;
    presence.game = &game;
    presence.game->name = "example-bot-hash";
    presence.game->type = LD_PRESENCE_LISTENING;
    presence.roles = NULL; /// \todo: presences should be initialized by a function

    info.init_presence = &presence;

    struct ld_context *context = malloc(sizeof(struct ld_context));


    if(context == NULL) {
        ld_error("example-bot-hash: couldn't initalize context");
        goto ABORT;
    }
    if(ld_init_context(&info, context) != 0) {
        ld_error("example-bot-hash: error initalizing context");
        goto ABORT;
    }

    int ret;
    int bot_state = 0; //not connected initially
    while(!bot_exit) {
        if(bot_state == 0) {
            ret = ld_connect(context);
            if(ret != 0) {
                ld_error("example-bot-hash: error connecting to discord (error %d)", ret);
                bot_exit = 1;
            }
            bot_state = 1;
        } else {
            ret = ld_service(context, 20);
            if(ret != 0) {
                ld_error("example-bot-hash: ld_service returned error (error %d)", ret);
                bot_exit = 1;
            }
        }
    }

    ld_send_basic_message(context, 345264084679131146, "example-bot-hash: received SIGINT or HCF, cleaning up");

    ABORT:
    ld_cleanup_context(context);
    free(bot_token);
    return 0;
}