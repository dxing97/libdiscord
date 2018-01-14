## URLs
All URLs defined by libdiscord are set at compile time and (eventually) can be changed in the CMake build file.
URLs will begin with ``https://`` or ``wss://`` and will not end with ``/``. Some libwebsockets context options require 
the `scheme://` part to be omitted.

## Connection states
Connections to Discord can't be characterized simply as "connected" and "disconnected". Connections include:
* the idempotent REST API
* the websocket API
* the UDP voice connection (not implemented yet)

A bot that's not connected to the gateway can still send API requests over the REST API as long as the bot
token has connected to and identified on the gateway at least once

### The gateway
Heartbeats are expected approximately every 42 seconds and can be delayed by up to 10 seconds
as of December 2017. This should not be treated as a hard rule. Since we have a leeway of 10 seconds,
using seconds as our precision for keeping track of heartbeat intervals shouldn't pose a problem in the near future.
If the margin of error shrinks, use ms to keep track of heartbeat intervals

User initiated disconnection from the gateway should be taken to mean closure of the bot (for now)

States:
* Connected
  * The bot is connected to Discord and everything is normal.
* Unconnected
  * The bot has never connected to Discord before, or we were disconnected and should start a new websocket connection
* Connecting (complicated)
  * multi-step process
  * If we've been disconnected previously, starting a new connection depends on how we were disconnected previously


## Gateway Initialization
Steps required to connect to discord:
* Verify the bot token and get a gateway URL with GET /gateway/bot
    * Callback: token verify (before and after)
    * What about bot sharding?
* Initialize a gateway connection
    * Gateway initialization and connection must start before trying to use the REST API because bots that have never 
    connected to the gateway may not be able to use all the features of the REST API.
    * prepare lws context and initialization
    * have callbacks to optionally modify websocket settings
    * Initialize the gateway send and receive queues (FILO)
    * Connect to the Discord websocket as a client
        * Recieve HELLO
        * Start heartbeating
        * Send IDENTIFY
        * note: HELLO/heartbeat and IDENTIFY/Ready don't have to be sent in a certain order.
        Just don't send a heartbeat before HELLO and don't send non-hb-related payloads before IDENTIFY
    
## User callback
A non-zero return value from the user callback will be taken to mean disconnect from the gateway.
Do state control within callbacks?