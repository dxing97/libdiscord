#libdiscord
A Discord library written in C, currently in development.

Discord server: https://discord.gg/BGgcQQh

The goal of this library currently is:
* be easy to use

## Building
See [BUILD.md](building)
Currently only tested against Ubuntu 17.10 Raspbian stretch, and the unofficial sparc64 port for Debian.

See [install.sh](install.sh) for a quick rundown of the commands you may have to run

Dependencies: 
* libwebsockets (tested to work with v2.4.1, compiled from source)
* jansson-dev (used package bundled with ubuntu 17.10)
* libcurl (used package bundled with ubuntu 17.10)
* getopt (required for ayylmao example bot, should be included with your *nix distribution)
* ulfius (for simple REST requests)

Tested to work on Raspberry Pi 3/Raspbian (stretch), but have to compile checkinstall for libwebsockets to be compiled. 

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

### example-kraken
This is what the real kraken should look like:
![The Real Kraken](https://assets.wired.com/photos/w_660/wp-content/uploads/2014/09/142-143-thekraken.jpg)

### references
For reference please see the following graphs:

![The Real Kraken](http://coveredtruths.com/wp-content/uploads/2017/03/top-10-real-aliens-caught-on-tape-2017-area-51-alien-footage-aliens-video.jpg)

![The Real Kraken](http://image.b4in.net/resources/2013/09/19/1379553235-GREY+ALIEN+IN+TUBE.jpg)
