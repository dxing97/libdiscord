/** @file */ 

#ifndef LIBDISCORD_STATUS_H
#define LIBDISCORD_STATUS_H

/**
 * @brief Status codes returned from libdiscord functions
 * @description
 * 0 to 99: OK, error, and memory issues
 * 100 to 199: generic connection issues
 * 200 to 299: libwebsocket issues
 * 300 to 399: JSON parsing issues (jansson, etc)
 * 900 to 999: Discord issues (tokens, permissions, etc.)
 */
 typedef enum ld_status_enum {
     LDS_OK = 0, ///< success
     LDS_ERROR = 1, ///< generic error
     LDS_MEMORY_ERR = 2, ///< generic memory error
     LDS_ALLOC_ERR = 3, ///< error allocating something (usually a struct)
     LDS_INCOMPLETE_ARGS_ERR = 4, ///< supplied arguments are incomplete (i.e. recieved null pointer when we weren't expecting one)INC
     LDS_CONNECTION_ERR = 100, ///< connection error
     LDS_CURL_ERR = 101, ///< CURL error
     LDS_WEBSOCKET_RINGBUFFER_ERR = 200, ///< generic ringbuffer error
     LDS_WEBSOCKET_RINGBUFFER_FULL_ERR = 201, ///< websocket: ringbuffer full
     LDS_WEBSOCKET_CANTFIT_PAYLOAD_ERR = 210, ///< websocket: can't fit new payload into ringbuffer error
     LDS_WEBSOCKET_CANTFIT_HEARTBEAT_ERR = 211, ///< websocket: can't fit heartbeat into ringbuffer error
     LDS_WEBSOCKET_HEARTBEAT_ACKNOWLEDEGEMENT_MISSED = 220, ///< websocket: heartbeat ack missed
     LDS_WEBSOCKET_INIT_ERR = 230, ///< websocket: error initializing lws context
     LDS_JSON_ERR = 300, ///< generic JSON error
     LDS_JSON_DECODING_ERR = 301, ///< JSON decoding error
     LDS_JSON_ENCODING_ERR = 302, ///< JSON encoding error
     LDS_JSON_DUMP_ERR = 303, ///< jansson error: couldn't dump json_t to string
     LDS_JSON_INVALID_ERR, ///< json struct failed to pass validation checks
     LDS_JSON_CANTFINDKEY_ERR = 310, ///< JSON no such key found error
     LDS_JSON_CANTFINDVALUE_ERR = 311, ///< JSON value expected, none found error
     LDS_JSON_MISSING_REQUIRED_ERR = 312, ///< JSON value required, none given/found error
     LDS_JSON_GATEWAY_DISPATCH_TYPE_ERR = 320, /// JSON gateway dispatch type not resolved error
     LDS_TOKEN_MISSING_ERR = 900 ///< no token specified for bot
 } ld_status;

 #endif //LIBDISCORD_STATUS_H