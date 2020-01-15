## Milestones
* v0.3
    * fix all bugs that prevent a bot from running indefinitely
        * add resuming logic 
* v1.0
    * 100% coverage of all API functions (websocket gateway, REST, voice websocket, voice UDP, v1.0 release)



## Todo
Ordered roughly in terms of priority.

### Websocket
* **Reconnection logic**
  * logging functions stop working on a curl error why?
* More detailed and documented callbacks
* Sharding
* merge ld_connect and ld_service
* zlib compression

### CMake
* OS detection
* Dependency checks/finding

### Library
* Improve doxygen documentation
* Improve logging faclilities (switch to LWS' logging functions entirely?)
* ~~helper function to verify if a bot token is valid without connecting to discord~~
* Consider changing library name to something less generic

### Installation and deployment 
* Dockerfile image for quick testing
* Debian package available by v0.3 release for arm, x86, sparc 
* Add a way to build dependencies from source and linking statically

### REST
* Nonblocking HTTP requests
* Ratelimiting
* Add convenience functions for logging bot actions (including internal library stuff) through a Discord channels (use Discord channel for logging)

### JSON
* **FIX JANSSON REFERENCES SO WE DON'T LEAK AS MUCH MEMORY**
* Differentiate between null and missing json fields in json structs
* JSON manipulation/creation functions for each type of JSON object that the API will send to us
* string to snowflake function
* Timestamp manipulation functions
    * Make/find functions that will encode/decode ISO8601-formatted strings. Use GNU function?
    
### Example Bots
* fold hash and counter into ayylmao (or remove entirely)

#### ayylmao - A Basic Call and Response Bot
* Add support for multiple calls/responses per bot
  * wew-lad, hmm-🤔 as default 
  
#### minimalws - Minimal example websocket bot
* Brings bot presence online
* Waits for stop message

#### hash - digest generator
* accept file uploads to hash
* spin off into its own repo seperate from the library repo, use to test libdiscord shared library config

#### counter - responds to i with i+1
* add halt message channel option
* possibly spin off into its own repo

#### tracker - Track user presence
* a bot to tell you when your bot ~~crashes~~ goes offline  (waitaminute...)

#### ping - A Comprehensive Latency Tool
* Measure communication latencies to Discord and internal bot delays and metrics
* Integrate into library as diagnostic functions

#### simplepost - REST-only simple message posting
* script-friendly app for posting simple messages


## Far Future
* Rich presence integration?
* OAuth2 support (need to write/find a OAuth2 client library for C)
* Support for voice channels (UDP connections, use libuv?)
* Support for file-based bot configuration allowing in-flight changing of bot behavior
* A Discord CLI client (probably as a separate app)
* Support for ESP32
* Support for Windows
* Make/autogenerate wrapper for other languages, including Python, C++, Rust, and Go.

## Far Far Future
* Discord as a Filesystem (DaaFS)