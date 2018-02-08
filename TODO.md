## Goals
* Get a stable working minimal example bot (done) with minimal spaghetti (almost there)
* Get 100% coverage of all non-voice API functions (v1.0 release)
* Make a bot that can turn my SSH lights on and off
* Discord developer ToS compliant

## Todo
* Try running a test bot on a high-end Intel server instead of Raspberry Pis. 
* Add wrapper for ulfius' u_map functions and adapt for nonblocking curl use
* A macro that loops through all _u_map members (like jansson's foreach object function)

### Websocket
* Detect gateway disconnection in ld_service and reconnect
* Implement websocket close code parsing (websocket standard codes like 1000 and 1001)
* user callbacks for both tx and rx payloads (specify difference between rx heartbeat and tx HB)

### REST
* Try using OpenSSL to see if our "unexpected TLS packet" error goes away
* Add way of logging bot actions through a Discord channels (use Discord channel for logging)
* Implement ratelimiting on REST API in conjunction with curl-multi (is this feasible?)
* parse HTTP headers for ratelimit info in /gateway/bot
* Look into Ulfius performance issues (excessive ``curl_easy_init`` and ``curl_easy_cleanup``)

### JSON
* JSON manipulation/creation functions
* Figure out how to use structs with json_t
* Consider adding a snowflake type

## Planned
* Git submodules to statically link some dependencies (cygwin)
* make example-ayylmao not respond to other bots
* zlib compression support in gateway payloads (check other compression methods in gateway connections)
* See if jansson increment/decrement needs to be used
* CMake OS detection for IDENTIFY payload
* Better documentation (comments in the source code is NOT good documentation)
* latency bot

## Far Future
* Rich presence integration for IoT
* Make ulfius optional, i.e. only use libcurl
* OAuth support (write/find a OAuth2 library for C)
* A command line application for scripting with bash
* Support for voice channels (UDP connections)
* Support for file-based bot configuration allowing in-flight changing of bot behavior
* Example bot with image generation (Allegro 5 bitmaps? MagickWand?)
* Support for userbots
* Example tool for a basic Discord CLI client.
* Figure out what Spotify is
* Make/autogenerate wrapper for other languages, including Python