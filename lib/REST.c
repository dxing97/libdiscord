//
// Created by dxing97 on 1/15/18.
//

#include "REST.h"

int ld_rest_init_request(struct ld_rest_request *request) {
    if(request == NULL) {
        ld_error("request pointer is NULL");
        return 1;
    }
    request->base_url = NULL;
    request->endpoint = NULL;
    request->verb = LD_REST_VERB_GET;
    request->body_size = 0;
    request->body = NULL;
    request->headers = NULL;
    request->timeout = 0;
    return 0;
}


int ld_rest_init_response(struct ld_rest_response *response) {
    if(response == NULL) {
        ld_error("response pointer is NULL");
        return 1;
    }
    response->http_response_code = 0;
    return 0;
}

/*
 *
 * rest blocking request, using ulfius
 * performance is not good, use if you need a response NOW
 * this function won't touch the memory you pass to it, so you will have to free it yourself
 */
int ld_rest_send_blocking_request(struct ld_rest_request *request, struct ld_rest_response *response) {
    /*
     * initialize a request
     * translate libdiscord request to an ulfius request
     */
    struct _u_request req;
    ulfius_init_request(&req);

    if(request->endpoint == NULL) {
        ld_error("send blocking request: endpoint is NULL!");
        return 1;
    }
    if(request->base_url == NULL) {
        ld_error("send blocking request: base url is NULL!");
    }
    char *url;
    url = malloc(strlen(request->base_url) + strlen(request->endpoint) + 1);
    url = strcpy(url, request->base_url);
    url = strcat(url, request->endpoint);

    struct _u_response resp;
    ulfius_init_response(&resp);

    int ret;

    ld_debug("blocking REST request URL: %s", url);

    req.http_verb = strdup(ld_rest_verb_enum2str(request->verb));
    req.http_url = url;
    req.binary_body = request->body;
    req.binary_body_length = request->body_size;
    req.map_header = ld_rest_ldh2umap(request->headers); //watch out for this
    req.timeout = request->timeout;

    ret = ulfius_send_http_request(&req, &resp);
    if(ret != U_OK) {
        ld_warning("couldn't send ulfius blocking HTTP request!");
        return 1;
    }

    free(url);
    ulfius_clean_request(&req);

    return 0;
}

/*
 * converts ld_headers to _u_map
 */
struct _u_map * ld_rest_ldh2umap(struct ld_headers *ldh) {
    return ldh->umap;
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
    return NULL;
}