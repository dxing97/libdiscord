## URLs
All URLs defined by libdiscord are set at compile time and (eventually) can be changed in CMakeLists.txt.
URLs will begin with ``https://`` or ``wss://`` and will not end with ``/``. Some libwebsockets context options require 
the `scheme://` part to be omitted. User code shouldn't have to worry about this.

## Connection states
~~Connections to Discord can't be characterized simply as "connected" and "disconnected".~~ Connections include:
* the idempotent REST API
* the websocket API (gateway in the API docs)
* the UDP voice connection

A bot that's not connected to the gateway can still send API requests over the REST API, with the only
exception being posting messages without having connected to the gateway before. 

### The gateway
Heartbeats are expected approximately every 42 seconds and can be delayed by up to 10 seconds
as of December 2017. This should not be treated as a hard rule. Since we have a leeway of 10 seconds,
using seconds as our precision for keeping track of heartbeat intervals shouldn't pose a problem in the near future.
If the margin of error shrinks, use ms to keep track of heartbeat intervals.

States:
* Connected
  * The bot is connected to the gateway and ready to receive events.
* Unconnected
  * The bot has never connected to Discord before, or we were disconnected and should start a new websocket connection
* Connecting
  * initiating a websocket connection
* Identifying
  * connected to the gateway, but haven't sent a IDENTIFY payload yet
* Resuming
  * connected to the gateway, but we haven't sent a RESUME payload yet
* Replaying
  * currently replaying events

## Gateway Initialization
Steps required to connect to discord:
* Verify the bot token and get a gateway URL with GET /gateway/bot
    * Callback: token verify (before and after)
    * What about bot sharding?
    
* Initialize the gateway send and receive queues (FILO)
* Initialize a gateway connection
    * prepare lws context and initialization
    * Connect to the Discord websocket as a client
        * Recieve HELLO payload
            * Start heartbeating
        * Send IDENTIFY
        * note: HELLO/heartbeat and IDENTIFY/Ready don't have to be sent in a certain order.
        Just don't send a heartbeat before HELLO and don't send non-hb-related payloads before IDENTIFY
    
## User callback
A non-zero return value from the user callback will usually be taken to mean disconnect from the gateway.

## REST API 
Simple interface for endpoint coded in libdiscord in pseudocode
```C
request = ld_rest_get_gateway_bot(context)
ld_rest_simple_perform(request, response)
//process response
```
```C
request = ld_create_basic_message(char *channel_id, message_t *message);
ld_queue_request(request);
ld_rest_simple_perform(request, response)
```

## JSON 
Some JSON objects/payloads are arrays of objects of some length. libdiscord will treat them as arrays of pointers to 
their respective struct type, and sets the last pointer to be a null pointer.

## REST API
There will be functions that generate requests, queue or perform requests, and parse responses

The Ulfius API makes this relatively straightforward, but incurs a performance penalty associated with not reusing connections.

There should be a built-in libcurl-based API that allows for the reusing of connections (have a pool of easy handles)

Similar to the curl interface, there should be an option to use non blocking and blocking requests. 

## 
```markdown
heartbeat_handler() {
    if there hasn't been a heartbeat response in a while
        disconnect with reason "HEART_ATTACK"
    if enough time has passed since the last heartbeat
        queue a new heartbeat
}

service() {
    switch(bot state) {
        case: not connected
            connect()
        case: connected
            heartbeat_handler()
            service(wsi)
}

main() { // bot event loop, user code
    context = initialize connection context
    while bot is on
        if(service(context) != OK) 
            bot is off
            
   cleanup(context)
}

```
