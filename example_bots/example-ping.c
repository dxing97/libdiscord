//
// Created by danielxing.6 on 11/21/2017.
//

/*
 * ping bot example
 *
 * potential response times to measure:
 *
 *      latency between sending a HTTP request and receiving a response for endpoints such as:
 *          GET /gateway
 *          DELETE [message]
 *          POST [message]
 *          GET [messages]
 *
 *      latency between a HTTP request and it's corresponding gateway dispatch, such as:
 *          POST [message] -> G RECIEVE DISPATCH new [message]
 *
 *      latency between a sent gateway payload and a response from the gateway
 *          G SEND STATUS_UPDATE -> G RECIEVE DISPATCH new [status]
 *          G SEND HEARTBEAT ->
 */
#include <libdiscord.h>

int main() {
    return 0;
}