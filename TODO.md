## Goals
* Get a stable working minimal example bot with minimal spaghetti (mostly working, still too much spaghet)
* Get 100% coverage of all API functions (v1.0 release)
* ~~Discord developer ToS compliant~~ compliance will be up to bot developers, although the library will try to include 
some convenience functions to make this easier
* A bot that can turn your lights on 

## Todo
Ordered roughly in terms of priority.

### Library
* libuv event loop for libdiscord, libwebsockets and libcurl
  * figure out how libuv works
* Better documentation (comments in the source code is NOT good documentation)

### Websocket
* Recover from websocket disconnections 
* More callbacks
* Sharding

### REST
* Nonblocking HTTP requests
* Ratelimiting
* Do we need ulfius?
    * Try running a test bot on a high-end Intel server instead of Raspberry Pis and see if using ulfius incurs a 
        noticible performance difference. 
    * Add wrapper for ulfius' u_map functions and adapt for nonblocking curl use
    * A macro that loops through all _u_map members (like jansson's foreach object function)
    * Look into Ulfius performance issues (excessive ``curl_easy_init`` and ``curl_easy_cleanup``)
* keep libcurl handles open - add some way of keeping connections open
* Try using OpenSSL to see if our "unexpected TLS packet" error goes away
* Add way of logging bot actions through a Discord channels (use Discord channel for logging)

### JSON
* JSON manipulation/creation functions for each type of JSON object that the API will send to us
* Consider changing the snowflake type to uint64_t
* string to snowflake function
* Timestamp manipulation functions
    * Make/find functions that will encode/decode ISO8601-formatted strings. Will GNU save us?
    
### Example Bots
#### ayylmao - A Basic Call and Response Bot
* Bug: space in response CLI option causes spam of the response
* Add support for multiple calls/responses per bot
  * add wew-lad
* Ignore calls from other bots

#### ping - A Comprehensive Latency Tool
* Measure latency between libcurl and ulfius

#### A Discord CLI Tool
* Shell scripting integration?

#### dbotc - A Bot for the Masses
* Look into YAML formatting
* YAML -> JSON converter? Support multiple config types

## Planned
* Git submodules to statically link some dependencies (cygwin)
* zlib compression support in gateway payloads (check other compression methods in gateway connections)
* See if jansson increment/decrement needs to be used
* More elaborate CMake setup for OS detection, dependency checks

## Far Future
* Rich presence integration for IoT
* OAuth support (write/find a OAuth2 library for C)
* A command line application for scripting with bash
* Support for voice channels (UDP connections)
* Support for file-based bot configuration allowing in-flight changing of bot behavior
* Example bot with image generation integration (Allegro 5 bitmaps? MagickWand? SDL?)
* Support for userbots
* A Discord CLI client (not necessarily as an example app - may be a separate app)
* Port to ESP32
* Support for MinGW, Cygwin
* Figure out what Spotify is
* Make/autogenerate wrapper for other languages, including Python, C++, Rust, and Go.

## Far Far Future
* Discord as a Filesystem (DaaFS)