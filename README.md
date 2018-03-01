# libdiscord
A Discord library written in C, currently in development.

Discord server: https://discord.gg/BGgcQQh

The goal of this library currently is:
* be easy to use

See [the todo list](TODO.md) for a list of features that will be implemented at some point.

## Building
See [building](doc/BUILDING.md)

Currently only tested against Ubuntu 17.10, Raspbian stretch, and the unofficial sparc64 port for Debian.

Tested to work on Raspberry Pi 3/Raspbian (stretch).

## Example Bots
### example-ayylmao
Currently this is the only example bot under development. 
This is the first working bot using libdiscord, and 
is currently under development.

Responds to every "ayy" with "lmao" in the same channel the "ayy" was sent in. Trigger and response can be set to other 
values by passing command line options.

### example-ping
Under consideration. Will try to roughly measure latencies and response times of various API endpoints, which may include:
* Response time of various REST API endpoints (GET /gateway, DELETE [message], etc)
* Response time of gateway heartbeats
* Response time of REST-gateway event pairs (POST [message] | recieved gateway dispatch for message event, etc)
* Response time to (user defined) third party servers (like a CLI ping tool)

