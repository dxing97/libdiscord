//
// Created by dxing97 on 11/11/17.
//

#include "libdiscord.h"

/*
 * This bot will respond to every "ayy" with "lmao"
 * todo: won't respond to other bots
 */

#include <getopt.h>
#include <signal.h>

static int bot_exit = 0;

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
 *
 */
int callback(struct ld_context *context, enum ld_callback_reason reason, json_t *data) {
    /*
     * depending on the reason, do stuff
     */
    //if the reason is a gateway event, and the payload is a dispatch, and the dispatch contains a message JSON, and the message has content "ayy" (case statement?)
        //post a "lmao" to the channel
        //create a new message info struct with content "lmao"
        //add that context to the send queue
    CURLcode res;
    const char *key, *content, *channelid;
    json_t *value;
    int ayystat = 0;

    switch(reason){
        case LD_CALLBACK_MESSAGE_CREATE:
            //if content == "ayy", POST "lmao" to that channel
            ld_info(context, "received MESSAGE_CREATE dispatch");
            //we want the "content" and "channel id" fields
            json_object_foreach(data, key, value) {
                if(strcmp(key, "content") == 0) {
                    content = json_string_value(value);
                    if(content == NULL) {
                        ld_warn(context, "couldn't get message content");
                        break;
                    }
                    if(strcmp(content, "ayy") == 0) {
                        ayystat++;
                    }

                }
                if(strcmp(key, "channel_id") == 0) {
                    channelid = json_string_value(value);
                    if(channelid == NULL) {
                        ld_warn(context, "couldn't get channel_id");
                        break;
                    }
                    ayystat++;

                }
            }
            break;
//        case LD_CALLBACK_MESSAGE_UPDATE:
//            return 1;
    }

    ld_info(context, "ayystat = %d", ayystat);

    if(ayystat != 2) { //did not get channel ID and ayy content
        return 0;
    }
    //generate POST message
    json_t *body;
    body = json_pack("{ss}", "content", "lmao");
    if(body == NULL) {
        ld_err(context, "couldn't create JSON object for lmao data");
        return 0;
    }
    char *tmp;
    tmp = json_dumps(body, 0);
    if(tmp == NULL) {
        ld_err(context, "couldn't dump JSON string for lmao data");
        return 0;
    }
    char *jsonbody = strdup(tmp);
    ld_debug(context, "body to post: %s", jsonbody);

    //curl POST to that channel
    struct curl_slist *headers = NULL;
    char auth_header[1000];
    sprintf(auth_header, "Authorization: Bot %s ", context->bot_token);
    headers = curl_slist_append(headers, auth_header);
    headers = curl_slist_append(headers, "Content-Type: application/json");

    char url[1000];
    sprintf(url, "%s/%s/messages", LD_API_URL LD_REST_API_VERSION "/channels", channelid);

    CURL *handle;
    handle = curl_easy_init();

    curl_easy_setopt(handle, CURLOPT_URL, url);
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "DiscordBot (https://github.com/dxing97/libdiscord 0.3)");
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, jsonbody);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDSIZE, (long) strlen(jsonbody));
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 1);

    res = curl_easy_perform(handle);
    if(res != CURLE_OK) {
        ld_err(context, "couldn't POST lmao");
    }

    free(jsonbody);
    curl_slist_free_all(headers);
    curl_easy_cleanup(handle);

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

    signal(SIGINT, int_handler);

    //define context info, including bot token
    struct ld_context_info *info;
    info = malloc(sizeof(struct ld_context_info));

    info->bot_token = strdup(bot_token);
    info->log_level = log_level;
    info->user_callback = callback;
    info->gateway_ringbuffer_size = 8; //todo: fine tune this value
    free(bot_token);

    //initialize context with context info
    struct ld_context *context;
    context = ld_create_context_via_info(info); //garbage in, garbage out
    if(context == NULL) {
        fprintf(stderr, "error creating ld context\n");
        return 1;
    }
    free(info);

    int ret, i = 0;
    //while the bot is still alive
    while(!bot_exit) {
        //if the bot isn't connected to discord, connect to discord
        //maybe the user shouldn't worry about whether or not we've connected here in the user code
        //move state stuff to reasons in the user callback
        switch (ld_gateway_connection_state(context)) {
            case LD_GATEWAY_CONNECTED:
                break;
            case LD_GATEWAY_CONNECTING:
                ld_service(context, 20);
                break;
            case LD_GATEWAY_DISCONNECTED:
                bot_exit = 1; //if we get disconnected let's quit for now
                break;
            case LD_GATEWAY_UNCONNECTED:
                ret = ld_connect(context);
                if(ret != 0) {
                    ld_warn(context, "error connecting to discord: error code %d", ret);
                    bot_exit = 1;
                }
                break;
            default:
                break;
        }
        ld_service(context, 20);
//        sleep(1);
    }
    //disconnect from discord gracefully
    ld_info(context, "disconnecting from discord");
    //destroy the context
    ld_destroy_context(context);
    return 0;
}
