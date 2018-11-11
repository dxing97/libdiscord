//
// Created by dxing97 on 1/15/18.
//

#include "REST.h"

struct ld_headers * ld_headers_init(struct ld_headers *headers) {
    void *ret1, *ret2;

    ret1 = malloc(sizeof(char *));
    if(ret1 == NULL) {
        ld_warning("ld_headers_init: couldn't malloc key");
        return NULL;
    }

    ret2 = malloc(sizeof(char *));
    if(ret2 == NULL) {
        ld_warning("ld_headers_init: couldn't malloc value");
        return NULL;
    }

    headers->length = 0;
    headers->key = ret1;
    headers->value = ret2;

    return headers;
}

int ld_headers_put(struct ld_headers *headers, char *key, char *value) {
    if(headers == NULL) {
        ld_debug("ld_headers_put: null header pointer");
        return 0;
    }
    if(key == NULL && value == NULL) { //put in nothing
        ld_debug("ld_headers_put: null key and value pointers");
        return 0;
    }

    void *ret1, *ret2;

    ret1 = realloc(headers->key, sizeof(char *) * (headers->length + 1));
    ret2 = realloc(headers->value, sizeof(char *) * headers->length + 1);

    if(ret1 == NULL) {
        ld_warning("ld_headers_put: couldn't realloc keys");
        return 1;
    }
    if(ret2 == NULL) {
        ld_warning("ld_headers_put: couldn't realloc values");
        return 1;
    }

    headers->key = ret1;
    headers->value = ret2;

    headers->key[headers->length] = strdup(key);
    headers->value[headers->length] = strdup(value);
    headers->length++;

    return 0;
}

//removes everything from the header, does not free headers struct itself
int ld_headers_clean(struct ld_headers *headers) {
    for(int i = 0; i < headers->length; i++) {
        free(headers->key[i]);
        free(headers->value[i]);
    }
    headers->length = 0;
    return 0;
}

int ld_headers2curl(struct ld_headers *headers, struct curl_slist **slist) {
    char tmp[2000];
    for (int i = 0; i < headers->length; ++i) {
        tmp[0] = '\0';
        strcat(tmp, headers->key[i]);
        strcat(tmp, ": ");
        strcat(tmp, headers->value[i]);
        *slist = curl_slist_append(*slist, tmp);
        if(*slist == NULL) {
            ld_warning("ld_headers2curl: error appending to slist");
            return 1;
        }
    }
    return 0;
}

struct ld_rest_request *ld_rest_init_request(struct ld_rest_request *request) {
//    struct ld_rest_request *request;
//    request = (struct ld_rest_request *)malloc(sizeof(struct ld_rest_request));
    if(request == NULL) {
        ld_warning("ld_rest_request: unexpected null request");
        return NULL;
    }
    request->base_url = NULL;
    request->endpoint = NULL;
    request->verb = LD_REST_VERB_GET;
    request->body_size = 0;
    request->body = NULL;
    request->headers = malloc(sizeof(struct ld_headers));
    request->headers = ld_headers_init(request->headers);
//    u_map_init(request->headers);
    request->timeout = 0;
    request->user_agent = NULL;

    return request;
}


struct ld_rest_response *ld_rest_init_response(struct ld_rest_response *response) {
//    struct ld_rest_response *response;
    response = (struct ld_rest_response *)malloc(sizeof(struct ld_rest_response));
    if(response == NULL) {
        ld_error("init response: allocated response pointer is NULL");
        return NULL;
    }
    response->http_status = -1;
    response->headers = (struct _u_map *)malloc(sizeof(struct ld_headers));
    if(response->headers == NULL) {
        ld_error("init response: error allocating headers");
        free(response);
        return NULL;
    }
//    if(u_map_init(response->headers) != U_OK) {
//        ld_error("init response: error initializing _u_map");
//        free(response->headers);
//        free(response);
//        return NULL;
//    }
    response->body = NULL;
    response->body_length = 0;

    return response;
}

int ld_rest_free_request(struct ld_rest_request *request){
    free(request->body);
    ld_headers_clean(request->headers);
    free(request->headers);
    free(request->base_url);
    free(request->endpoint);
    free(request);
    return LDE_OK;
}

int ld_rest_free_response(struct ld_rest_response *response){
//    int ret;
//    ret = u_map_clean_full(response->headers);
//    if(ret != U_OK) {
//        ld_warning("free response: couldn't free response headers");
//        return LDE_MEM;
//    }
    free(response);
    return LDE_OK;
}

/*
 *
 * rest blocking request, using ulfius
 * performance is not good if you need to do a lot of REST requests
 * this function won't touch the memory you pass to it, so you will have to free it yourself
 */
int
ld_rest_send_request(struct ld_context *context, struct ld_rest_response *response, struct ld_rest_request *request) {
    /*
     * initialize a request
     * translate libdiscord request to an ulfius request
     */
//    struct ld_rest_request req;
//    ld_rest_init_request(&req);

    //if the "endpoint" is null, is that really an error?
//    if(request->endpoint == NULL) {
//        ld_error("send blocking request: endpoint is NULL!");
//        return 1;
//    }
    if(request->base_url == NULL) {
        ld_error("send blocking request: base url is NULL!");
        return LDE_MISSING_PARAM;
    }
    char *url;
    url = malloc(strlen(request->base_url) + strlen(request->endpoint) + 1);
    url = strcpy(url, request->base_url);
    url = strcat(url, request->endpoint);

    struct ld_rest_response resp;
    ld_rest_init_response(&resp);
    struct curl_slist *slist;


    ld_headers2curl(request->headers, &slist);

    int ret;
//    char url[1000];
//    sprintf(url, "%s%s", request->base_url, request->endpoint);

    curl_easy_setopt(context->curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(context->curl_handle, CURLOPT_HTTPHEADER, slist);
    curl_easy_setopt(context->curl_handle, CURLOPT_USERAGENT, "DiscordBot (https://github.com/dxing97/libdiscord 0.3)");
    curl_easy_setopt(context->curl_handle, CURLOPT_POSTFIELDS, request->body);
    curl_easy_setopt(context->curl_handle, CURLOPT_POSTFIELDSIZE, (long) request->body_size);
    curl_easy_setopt(context->curl_handle, CURLOPT_VERBOSE, 1);

    ret = curl_easy_perform(context->curl_handle);
    if(ret != CURLE_OK) {
        ld_warning("ld_rest_send_request: curl_easy_perform returned error");
    }

    //todo: add response part

//    ld_debug("blocking REST request URL: %s", url);

//    req.http_verb = strdup(ld_rest_verb_enum2str(request->verb));
//    req.http_url = url;
//    req.binary_body = request->body;
//    req.binary_body_length = request->body_size;
//    req.map_header = request->headers; //watch out for this
//    req.timeout = request->timeout;

//    ret = ulfius_send_http_request(&req, &resp);
//    if(ret != U_OK) {
//        ld_warning("ulfius: couldn't send ulfius blocking HTTP request! (%d)", ret);
//        return LDE_ULFIUS;
//    }
//
//    response->http_status = resp.status;
//    response->body = strndup(resp.binary_body, resp.binary_body_length);
//    response->body_length = resp.binary_body_length;
//
//    free(url);
//
    return LDE_OK;
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
    req->user_agent = strdup(tmp);

    req->verb = LD_REST_VERB_GET;

    return req;
}

struct ld_rest_request *ld_get_gateway_bot(struct ld_context *context, struct ld_rest_request *req) {
    char tmp[1000];

    sprintf(tmp, "%s%s", LD_API_URL, LD_REST_API_VERSION);
    req->base_url = strdup(tmp);

    req->endpoint = strdup("/gateway/bot");

    sprintf(tmp, "DiscordBot (%s %s)", LD_GITHUB_URL, LD_VERSION);
    req->user_agent = strdup(tmp);

    req->verb = LD_REST_VERB_GET;
    sprintf(tmp, "Bot %s", context->bot_token);
    ld_headers_put(req->headers, "Authorization", tmp);
//    u_map_put(req->headers, "Authorization", tmp);
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
    ld_headers_put(req->headers, "Authorization", tmp);

    //generate POST message
    json_t *body;

    body = json_pack("{ss}", "content", message_content);
    if(body == NULL) {
        ld_error("couldn't create JSON object for lmao data");
        return LDE_JSON;
    }

    char *json_body;

    json_body = json_dumps(body, 0);
    if(json_body == NULL) {
        ld_error("couldn't dump JSON string for lmao data");
        return LDE_JSON;
    }
    json_body = strdup(json_body);

    ld_debug("body to post: %s", json_body);

    req->body = json_body;
    req->body_size = strlen(req->body);

    return LDE_OK;
}