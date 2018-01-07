## Goals
* Get a stable working minimal example bot (done) with minimal spaghetti (almost there)
* Get 100% coverage of all non-voice API functions (v0.3 point release)
* Make a bot that can turn my SSH lights on and off
* Discord developer ToS compliant

## Todo
* Use nonzero return value from user callback to mean disconnect from gateway(??)
* Implement ring buffer for gateway payloads
* Handle gateway-side disconnections elegantly
* Allow user code to specify in IDENTIFY payload parameters like game name/status and OS
* Add nonblocking REST implementation using curl-multi

## Planned
* See if jansson increment/decrement needs to be used
* Implement more user callbacks
* CMake OS detection for IDENTIFY payload
* Find a better way of logging stuff/make logging better
* Better documentation (comments in the source code is NOT good documentation)
* latency bot

## Far Future
* Rich presence integration for IoT
* A command line application for scripting
* Support for voice channels (UDP connections)
* Support for file-based bot configuration allowing in-flight changing of bot behavior
* Example bot with image generation (Allegro 5 bitmaps? MagickWand?)
* Example tool for a basic Discord CLI client.