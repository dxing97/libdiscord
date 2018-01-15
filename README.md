### libdiscord
A Discord library written in C.

The goal of this library currently is:
* be easy to use

## Building
See [BUILD.md](building)
Currently only tested against Ubuntu 17.10 and Raspbian stretch.

See [install.sh](install.sh) for a quick rundown of the commands you may have to run


## Example Bots
### example-ayylmao
Currently this is the only example bot under development. 
This is the first working bot using libdiscord, and 
is currently under development.

Responds to every "ayy" with "lmao" in the same channel the "ayy" was sent in.

### example-ping
Under consideration. Will try to roughly measure latencies and response times of various API endpoints, which may include:
* Response time of various REST API endpoints (GET /gateway, DELETE [message], etc)
* Response time of gateway heartbeats
* Response time of REST-gateway event pairs (POST [message] | recieved gateway dispatch for message event, etc)
* Response time to (user defined) third party servers (like a CLI ping tool)

