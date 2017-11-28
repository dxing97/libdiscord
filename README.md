### libdiscord
A Discord library written in C.

The goals of this library currently are:
* be easy to use

## Example Bots
### example-ayylmao
Currently this is the only example bot under development. This will be the first working bot using libdiscord, and 
is currently under development.

Responds to every "ayy" with "lmao".

### example-ping
Under consideration. Will measure latencies and response times of various API endpoints, which may include:
* Response time of various REST API endpoints (GET /gateway, DELETE [message], etc)
* Response time of gateway heartbeats
* Response time of REST-gateway event pairs (POST [message] | recieved gateway dispatch for message event, etc)
* Response time to (user defined) third party servers
