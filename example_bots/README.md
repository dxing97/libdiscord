# Example Bots

### example-ayylmao

Responds to every "ayy" with "lmao" in the same channel the "ayy" was sent in. Trigger and response can be set to other 
values through command line options.

### example-testing

REST-only bot, POSTs a simple message to a channel then exits. Channel ID and message are specified via command line options.

Useful for simple shell scripts.

### example-ping
Under development. Will try to roughly measure latencies and response times of various API endpoints, which may include:
* Response time of various REST API endpoints (GET /gateway, DELETE [message], etc)
* Response time of gateway heartbeats
* Response time of REST-gateway event pairs (POST [message] | recieved gateway dispatch for message event, etc)
* Response time to (user defined) third party servers (like a CLI ping tool)