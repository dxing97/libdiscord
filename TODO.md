## Goals
* Get a stable working minimal example bot (done) with minimal spaghetti (almost there)
* Get 100% coverage of all non-voice API functions (v1.0 release)
* Make a bot that can turn my SSH lights on and off
* Discord developer ToS compliant

## Todo
* Use ulfius for blocking REST functions
* start moving stuff out of libdiscord.h into their own separate functions
* Make a distinction between different websocket connections/shards using wsi user pointers
* Implement ratelimiting on REST API in conjunction with curl-multi (is this feasible?)
* parse HTTP headers for ratelimit info in /gateway/bot
* Implement websocket close code parsing (websocket standard codes like 1000 and 1001)
* Rethink connection states
* user callbacks for both tx and rx payloads (specify difference between rx heartbeat and tx HB)

## Planned
* Add REST API functions
* make example-ayylmao not respond to other bots
* zlib compression support in gateway payloads
* See if jansson increment/decrement needs to be used
* OAuth support
* CMake OS detection for IDENTIFY payload
* Better documentation (comments in the source code is NOT good documentation)
* latency bot

## Far Future
* Rich presence integration for IoT
* A command line application for scripting
* Support for voice channels (UDP connections)
* Support for file-based bot configuration allowing in-flight changing of bot behavior
* Example bot with image generation (Allegro 5 bitmaps? MagickWand?)
* Example tool for a basic Discord CLI client.