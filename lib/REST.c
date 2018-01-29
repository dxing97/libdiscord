//
// Created by dxing97 on 1/15/18.
//

#include "REST.h"

struct ld_rest_request * ld_rest_init_request() {
    struct ld_rest_request *request;
    request = (struct ld_rest_request *)malloc(sizeof(struct ld_rest_request));
    if(request == NULL) {
        ld_error("couldn't allocate ld_rest_request");
        return NULL;
    }
    request->base_url = NULL;
    request->endpoint = NULL;
    request->verb = LD_REST_VERB_GET;
    request->body_size = 0;
    request->body = NULL;
    request->headers = malloc(sizeof(struct _u_map));
    u_map_init(request->headers);
    request->timeout = 0;
    request->user_agent = NULL;

    return request;
}


struct ld_rest_response * ld_rest_init_response() {
    struct ld_rest_response *response;
    response = (struct ld_rest_response *)malloc(sizeof(struct ld_rest_response));
    if(response == NULL) {
        ld_error("init response: allocated response pointer is NULL");
        return NULL;
    }
    response->http_status = -1;
    response->headers = (struct _u_map *)malloc(sizeof(struct _u_map));
    if(response->headers == NULL) {
        ld_error("init response: error allocating headers");
        free(response);
        return NULL;
    }
    if(u_map_init(response->headers) != U_OK) {
        ld_error("init response: error initializing _u_map");
        free(response->headers);
        free(response);
        return NULL;
    }
    response->body = NULL;
    response->body_length = 0;

    return response;
}

int ld_rest_free_request(struct ld_rest_request *request){
    free(request->body);
    u_map_clean_full(request->headers);
    free(request->base_url);
    free(request->endpoint);
    free(request);
    return 0;
}

int ld_rest_free_response(struct ld_rest_response *response){
    int ret;
    ret = u_map_clean_full(response->headers);
    if(ret != U_OK) {
        ld_warning("free response: couldn't free response headers");
        return 1;
    }
    free(response);
    return 0;
}

/*
 *
 * rest blocking request, using ulfius
 * performance is not good if you need to do a lot of REST requests
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

//    ld_debug("blocking REST request URL: %s", url);

    req.http_verb = strdup(ld_rest_verb_enum2str(request->verb));
    req.http_url = url;
    req.binary_body = request->body;
    req.binary_body_length = request->body_size;
    req.map_header = request->headers; //watch out for this
    req.timeout = request->timeout;

    ret = ulfius_send_http_request(&req, &resp);
    if(ret != U_OK) {
        ld_warning("ulfius: couldn't send ulfius blocking HTTP request! (%d)", ret);
        return 1;
    }

    response->http_status = resp.status;
    response->body = strndup(resp.binary_body, resp.binary_body_length);
    response->body_length = resp.binary_body_length;
    //todo: make request and response allocation and deallocation functions

    free(url);
//    ulfius_clean_request(&req);

    return 0;
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

struct ld_rest_request *ld_get_gateway(struct ld_rest_request *req, struct ld_context *context) {
    char tmp[1000];

    sprintf(tmp, "%s%s", LD_API_URL, LD_REST_API_VERSION);
    req->base_url = strdup(tmp);

    req->endpoint = strdup("/gateway");

    sprintf(tmp, "DiscordBot (%s %s)", LD_GITHUB_URL, LD_VERSION);
    ld_debug("user-agent: %s", tmp);
    req->user_agent = strdup(tmp);

    req->verb = LD_REST_VERB_GET;

    return req;
}

struct ld_rest_request *ld_get_gateway_bot( struct ld_rest_request *req, struct ld_context *context) {
    char tmp[1000];

    sprintf(tmp, "%s%s", LD_API_URL, LD_REST_API_VERSION);
    req->base_url = strdup(tmp);

    req->endpoint = strdup("/gateway/bot");

    sprintf(tmp, "DiscordBot (%s %s)", LD_GITHUB_URL, LD_VERSION);
    ld_debug("user-agent: %s", tmp);
    req->user_agent = strdup(tmp);

    req->verb = LD_REST_VERB_GET;
    sprintf(tmp, "Bot %s", context->bot_token);
    u_map_put(req->headers, "Authorization", tmp);
    return req;
}
/*
 * message_content is the actual content of the message, not the HTTP body
 */
int ld_create_message(struct ld_rest_request *req,
                      struct ld_context *context,
                      const char *channel_id,
                      const char *message_content){
    char tmp[1000];

    sprintf(tmp, "%s%s", LD_API_URL, LD_REST_API_VERSION);
    req->base_url = strdup(tmp);

    sprintf(tmp, "/channels/%s/messages", channel_id);
    req->endpoint = strdup(tmp);

    sprintf(tmp, "DiscordBot (%s %s)", LD_GITHUB_URL, LD_VERSION);
    ld_debug("user-agent: %s", tmp);
    req->user_agent = strdup(tmp);

    req->verb = LD_REST_VERB_POST;
    sprintf(tmp, "Bot %s", context->bot_token);
    u_map_put(req->headers, "Authorization", tmp);

    //generate POST message
    json_t *body;

    body = json_pack("{ss}", "content", message_content);
    if(body == NULL) {
        ld_error("couldn't create JSON object for lmao data");
        return 1;
    }

    char *json_body;

    json_body = json_dumps(body, 0);
    if(json_body == NULL) {
        ld_error("couldn't dump JSON string for lmao data");
        return 1;
    }
    json_body = strdup(json_body);

    ld_debug("body to post: %s", json_body);

    req->body = json_body;
    req->body_size = strlen(req->body);

    return 0;
}