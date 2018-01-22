//
// Created by dxing97 on 1/15/18.
//

#include "REST.h"

struct ld_rest_request *ld_rest_init_request() {
    struct ld_rest_request *req;
    req = malloc(sizeof(struct ld_rest_request));
    if(req == NULL) {
        ld_error("couldn't allocate memory for restrequest");
        return NULL;
    }
    req->base_url = NULL;
    return req;
}

/*
 *
 * rest blocking request, using ulfius
 * performance is not good, use if you need a response NOW
 * this function won't touch the memory you pass to it, so you will have to free it yourself
 */
int ld_rest_blocking_request(struct ld_rest_request *request, struct ld_rest_response *response) {
    /*
     * initialize a request
     * translate libdiscord request to an ulfius request
     */
    struct _u_request *req;
    req = malloc(sizeof(struct _u_request));
    ulfius_init_request(req);

    char *url;
    url = malloc(strlen(request->base_url) + strlen(request->endpoint) + 1);
    url = strcpy(url, request->base_url);
    url = strcat(url, request->endpoint);

    ld_debug("blocking REST request URL: %s", url);

    req->http_verb = strdup(ld_rest_verb_enum2str(request->verb));
    req->http_url = url;
    req->binary_body = request->body;
    req->binary_body_length = request->body_size;


    free(url);
    url = NULL;
    ulfius_clean_request(req);
    req = NULL;
}

char *ld_rest_verb_enum2str(enum ld_rest_http_verb verb) {
    switch(verb) {
        case LD_REST_VERB_GET:
            return "GET";
        case LD_REST_VERB_PUT:
            return "PUT";
        case LD_REST_VERB_POST:
            return "POST";
        case LD_REST_VERB_PATCH:
            return "PATCH";
        case LD_REST_VERB_DELETE:
            return "DELETE";
    }
}