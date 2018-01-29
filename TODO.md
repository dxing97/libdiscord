## Goals
* Get a stable working minimal example bot (done) with minimal spaghetti (almost there)
* Get 100% coverage of all non-voice API functions (v1.0 release)
* Make a bot that can turn my SSH lights on and off
* Discord developer ToS compliant

## Todo
* Use ulfius for blocking REST 
    * write free functions for request and response
    * rewrite ld_get_gateway_bot and ld_get_gateway using
    ulfius
* Add wrapper for ulfius' u_map functions and adapt for nonblocking curl use
* make example bots comparing ulfius/frequent curl_easy_inits/frees and keeping a handle open
* Add defines or enums for return codes
* start moving stuff out of libdiscord.h into their own separate functions
* A macro that loops through all _u_map members (like jansson's foreach object function)
* Make a distinction between different websocket connections/shards using wsi user pointers
* Implement ratelimiting on REST API in conjunction with curl-multi (is this feasible?)
* parse HTTP headers for ratelimit info in /gateway/bot
* Implement websocket close code parsing (websocket standard codes like 1000 and 1001)
* Rethink connection states
* user callbacks for both tx and rx payloads (specify difference between rx heartbeat and tx HB)

## Planned
* Git submodules to statically link some dependencies (cygwin)
* Add REST API functions
* make example-ayylmao not respond to other bots
* zlib compression support in gateway payloads (check other compression methods in gateway connections)
* See if jansson increment/decrement needs to be used
* CMake OS detection for IDENTIFY payload
* Better documentation (comments in the source code is NOT good documentation)
* latency bot


## Far Future
* Rich presence integration for IoT
* OAuth support (write a OAuth2 library for C)
* A command line application for scripting
* Support for voice channels (UDP connections)
* Support for file-based bot configuration allowing in-flight changing of bot behavior
* Example bot with image generation (Allegro 5 bitmaps? MagickWand?)
* Example tool for a basic Discord CLI client.