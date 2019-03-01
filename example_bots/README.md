# Example Bots

### example-ayylmao

Responds to every "ayy" with "lmao" in the same channel the "ayy" was sent in. Trigger and response can be set to other 
values through command line options.

### example-counter and example-hash
Variations on ``example-ayylmao`` which do some kind of data processing. 

``example-counter`` will look for any integer ``x`` and respond with ``x+1``.

``example-hash`` will use OpenSSL to compute a user-specified digest of an arbitrary string.

### example-testing

REST-only bot, POSTs a simple message to a channel then exits. Channel ID and message are specified via command line options.

Useful for simple shell scripts.

### example-ping
Under development (not working yet). Will try to roughly measure latencies and response times of various API endpoints, which may include:
* Response time of various REST API endpoints (GET /gateway, DELETE [message], etc)
* Response time of gateway heartbeats
* Response time of REST-gateway event pairs (POST [message] | recieved gateway dispatch for message event, etc)
* Response time to (user defined) third party servers (like a CLI ping tool)