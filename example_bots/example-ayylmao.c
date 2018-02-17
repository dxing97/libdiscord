//
// Created by dxing97 on 11/11/17.
//

#include "libdiscord.h"

/*
 * This bot will respond to every "ayy" with "lmao"
 */

#include <getopt.h>
#include <signal.h>
#include <REST.h>

static int bot_exit = 0; //0: no exit, 1: exit
static int bot_state = 0; //0: not connected/disconnected, 1: connect initiated
CURL *handle;
int use_ulfius = 0;
char *trigger = NULL, *response = NULL;

void int_handler(int i){
    bot_exit = 1;
}

/*
 * main way of user interaction with libdiscord
 * the user callback returns 0 if everything is OK
 *
 * ld_context contains info about the bot. The user shouldn't have to mess with it.
 * ld_callback_reason is the reason for the library calling the callback. See the enum declaration in libdiscord.h
 * data and len contain data that may be needed for the callback, their use depends on the reason for the callback.
 */
int callback(struct ld_context *context, enum ld_callback_reason reason, void *data, int len) {
    /*
     * depending on the reason, do stuff
     */
    //if the reason is a gateway event, and the payload is a dispatch, and the dispatch contains a message JSON, and the message has content "ayy" (case statement?)
        //post a "lmao" to the channel
        //create a new message info struct with content "lmao"
        //add that context to the send queue
    CURLcode res;
    const char *key, *content, *channelid;
    int ayystat = 0, ret = 0;

    switch(reason){
        case LD_CALLBACK_MESSAGE_CREATE: {
            json_t *jdata = data, *value;

            //if content == "ayy", POST "lmao" to that channel
            ld_debug("received MESSAGE_CREATE dispatch");

            //we want the "content" and "channel id" fields
            json_object_foreach(jdata, key, value) {
                if (strcmp(key, "content") == 0) {
                    content = json_string_value(value);
                    if (content == NULL) {
                        ld_warning("couldn't get message content");
                        break;
                    }
                    if(strcasecmp(content, (trigger ? trigger : "ayy")) == 0 ) {
                            ayystat++;
                    }
                }

                if (strcmp(key, "channel_id") == 0) {
                    channelid = json_string_value(value);
                    if (channelid == NULL) {
                        ld_warning("couldn't get channel_id");
                        break;
                    }
                    ayystat++;
                }
            }
        }
            break;
        default:
            break;
    }

    ld_debug("ayystat = %d", ayystat);

    if(ayystat != 2) { //did not get channel ID and ayy content
        return 0;
    }
    //generate POST message
    json_t *body;
    body = json_pack("{ss}", "content", (response ? response : "lmao"));
    if(body == NULL) {
        ld_error("couldn't create JSON object for lmao data");
        return 0;
    }
    char *tmp;
    tmp = json_dumps(body, 0);
    if(tmp == NULL) {
        ld_error("couldn't dump JSON string for lmao data");
        return 0;
    }
    char *jsonbody = strdup(tmp);
    ld_debug("body to post: %s", jsonbody);

    if(use_ulfius) {
        struct ld_rest_request *request;
        struct ld_rest_response *resp;
        request = ld_rest_init_request();
        resp = ld_rest_init_response();

        ld_create_message(request, context, channelid, (response ? response : "lmao"));
        ld_rest_send_blocking_request(request, resp);
    } else {
        //curl POST to that channel
        struct curl_slist *headers = NULL;
        char auth_header[1000];
        sprintf(auth_header, "Authorization: Bot %s ", context->bot_token);
        headers = curl_slist_append(headers, auth_header);
        headers = curl_slist_append(headers, "Content-Type: application/json");



        char url[1000];
        sprintf(url, "%s/%s/messages", LD_API_URL LD_REST_API_VERSION "/channels", channelid);

        curl_easy_setopt(handle, CURLOPT_URL, url);
        curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(handle, CURLOPT_USERAGENT, "DiscordBot (https://github.com/dxing97/libdiscord 0.3)");
        curl_easy_setopt(handle, CURLOPT_POSTFIELDS, jsonbody);
        curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, (long) strlen(jsonbody));
        curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);
        ret = curl_easy_setopt(handle, CURLOPT_TCP_KEEPALIVE, 1L);
        if(ret != CURLE_OK) {
            ld_debug("TCP keepalive unavailable");
        }
        ret = curl_easy_setopt(handle, CURLOPT_TCP_KEEPIDLE, 120L);
        if(ret != CURLE_OK) {
            ld_debug("TCP keepalive idle unavailable");
        }
        ret = curl_easy_setopt(handle, CURLOPT_TCP_KEEPINTVL, 60L);
        if(ret != CURLE_OK) {
            ld_debug("TCP keepalive interval unavailable");
        }

        //todo: add a way to print the response

        res = curl_easy_perform(handle);
        if(res != CURLE_OK) {
            ld_error("couldn't POST lmao");
            return 0; //todo: possibly retry the POST if it fails
        }


        curl_slist_free_all(headers);
    }

    free(jsonbody);
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
    int c; //bot_exit: 0 for don't exit, 1 for exit
    char *bot_token = NULL;
    char *game = NULL;
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
                {"use-ulfius", no_argument, 0, 'u'},
                {"game", required_argument, 0, 'g'},
                {"trigger", required_argument, 0, 'r'},
                {"response", required_argument, 0, 'R'},
                {0,0,0,0}
        };

        int option_index = 0;
        c = getopt_long(argc, argv, "ht:l:ug:r:R:", long_options, &option_index);

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
                               "-u, --use-ulfius\n\t\t"
                               "If set, uses libulfius to send messages instead of libcurl. Default is to use libcurl\n\t"
                               "-g, --game\n\t\t"
                               "Sets the initial value of the \"game\" field in the bot presence.\n\t"
                               "-r, --trigger [trigger_string]\n\t\t"
                               "Sets string that will trigger a response from the bot. Default is \"ayy\".\n\t"
                               "-R, --response [response_string]\n\t\t"
                               "Sets response that will be sent when the trigger is read. Default is \"lmao\".\n\t"
                               "-h, --help\n\t\t"
                               "Displays this help dialog\n", argv[0]);
                return 0;
            case 't':
                bot_token = strdup(optarg);
                break;
            case 'l':
                log_level = strtoul(optarg, NULL, 10);
                break;
            case 'u':
                use_ulfius = 1;
                break;
            case 'g':
                game = strdup(optarg);
                break;
            case 'r':
                trigger = strdup(optarg);
                break;
            case 'R':
                response = strdup(optarg);
                break;
            default:
                abort();
        }
    }

    printf("Example bot 1 \"ayylmao\" starting up using libdiscord v%s\n", LD_VERSION);

    if(bot_token == NULL){
        printf("Bot token not set! See example-ayylmao -h for details.");
        return 1;
    }
    printf("Initializing libdiscord with log level %lu\n", log_level);

    ld_set_logging_level(log_level);

    signal(SIGINT, int_handler);

    //define context info, including bot token
    struct ld_context_info *info;
    info = malloc(sizeof(struct ld_context_info));

    struct _ld_json_presence *presence; //todo: presence currently cannot be null, fix this
    presence = malloc(sizeof(struct _ld_json_presence));
    presence->status_type = LD_PRESENCE_DND;
    presence->game_type = LD_PRESENCE_STREAMING;
    if(game != NULL) {
        presence->game = strdup(game);
    } else {
        presence->game = strdup("AlienSimulater");
    }

    info->init_presence = presence;
    info->bot_token = strdup(bot_token);
    info->log_level = log_level;
    info->user_callback = callback;
    info->gateway_ringbuffer_size = 8;

    free(bot_token);

    //initialize context with context info
    struct ld_context *context;
    context = ld_create_context_via_info(info); //garbage in, garbage out
    if(context == NULL) {
        ld_error("error creating libdiscord context\n");
        return 1;
    }

    free(presence->game);
    presence->game = NULL;
    free(game);
    game = NULL;
    free(info);
    free(presence);

    handle = curl_easy_init();

    int ret, i = 0;
    //while the bot is still alive
    while(!bot_exit) {
        if(bot_state == 0) {
            //bot isn't connected, so we should try connecting
            ret = ld_connect(context);
            if(ret != 0) {
                ld_warning("error connecting to discord: error code %d", ret);
                break;
            }
            bot_state = 1;
        }

        ret = ld_service(context, 20); //service the connection
        if(ret != 0) {
            ld_error("ld_service returned non-0 (%d)", ret);
            break;
        }
    }
    //disconnect from discord gracefully
    ld_info("disconnecting from discord");
    //destroy the context
    ld_destroy_context(context);
    curl_easy_cleanup(handle);
    return 0;
}
