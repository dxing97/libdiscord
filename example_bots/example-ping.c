//
// Created by danielxing.6 on 11/21/2017.
//

/*
 * ping bot example
 *
 * potential response times to measure:
 *
 *      latency between sending a HTTP request and receiving a response for endpoints such as:
 *
 *          GET /gateway
 *          DELETE [message]
 *          POST [message]
 *          GET [messages]
 *
 *          for both blocking (ulfius) and nonblocking (libcurl) methods
 *
 *
 *      latency between a HTTP request and it's corresponding gateway dispatch, such as:
 *          POST [message] -> G RX DISPATCH new [message]
 *
 *      latency between a sent gateway payload and a response from the gateway
 *          G TX STATUS_UPDATE -> G RX DISPATCH new [status]
 *          G TX HEARTBEAT -> G RX HEARTBEAT_ACK
 */
#include <libdiscord.h>

int main(int argc, char *argv[]) {
    ld_set_logging_level(31);

    ld_info("does nothing for the moment");
    return 0;
}