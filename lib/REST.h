//
// Created by dxing97 on 1/15/18.
//

#include <curl/curl.h>
#include <ulfius.h>
#include "libdiscord.h"

#ifndef LIBDISCORD_REST_H
#define LIBDISCORD_REST_H

/*
 * functions related to the REST API
 *
 */

/*
 * HTTP verb enums.
 */
enum ld_rest_http_verb {
    LD_REST_VERB_GET,
    LD_REST_VERB_PUT,
    LD_REST_VERB_POST,
    LD_REST_VERB_PATCH,
    LD_REST_VERB_DELETE
};

/*
 * http request fields
 */
struct ld_rest_request {
    enum ld_rest_http_verb verb;
    char *base_url;
    char *endpoint;
    char *user_agent;
    struct curl_slist headers; //headers
    char *body;
    size_t body_size;
};

/*
 * key-value pairs for HTTP headers
 */
struct ld_rest_header_line {
    char *name;
    char *value;
};

struct ld_rest_response {
    unsigned int http_response_code;
};

/*
 * creates a new request with some basic defaults for discord
 */
struct ld_rest_request *ld_rest_init_request();

void ld_rest_request_add_headers(struct ld_rest_request);

/*
 * takes a request, performs it, then saves the response
 * returns 0 on successful request (even if the response is 4XX)
 * rather slow, since it creates and destroys a curl handle.
 */
int ld_rest_blocking_request(struct ld_rest_request *request, struct ld_rest_response *response);

/*
 * makes a HTTP request of some kind to some URL with some headers
 * blocking
 */
int _ld_rest_blocking_request(
        enum ld_rest_http_verb verb,
        char *url,
        struct curl_slist headers,
        char *user_agent
);

int ld_get_gateway_bot();
int ld_post_channel_message();
#endif //LIBDISCORD_REST_H
