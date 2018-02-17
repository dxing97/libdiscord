# Building libdiscord

libdiscord uses the CMake build system, so building and installing should be fairly straightforward and relatively 
portable.

Dependencies: 
* libwebsockets 
    * minimum required version is v2.4, anything below that will not work, compiled from source)
* jansson (used package bundled with ubuntu, version 2.9 or later should work, v2.7 has been confirmed to NOT work
(json_pack changed behavior))
* libcurl (used package bundled with ubuntu 17.10, any recent version should work)
* ulfius (for simple REST requests, any recent version should work)

Note that each package has their own dependencies, including 

Tested to buuld and work on Raspberry Pi 3 and Raspberry Pi W running Raspbian (stretch), 
but note that if you want to install libwebsockets with checkinstall on Raspbian, 
then you will have to compile checkinstall yourself. Otherwise, install with ``sudo make install``

Works on sparc64 debian 4.13.4-1 with no issues currently.

### Quick build instructions for libwebsockets

Make sure you have the dependencies for libwebsockets installed.

Get a copy of libwebsockets source from github.
```bash
git clone https://github.com/warmcat/libwebsockets 
```
Checkout the right version (2.4.1)
```bash
cd libwebsockets
git checkout v2.4.1
```
Create a seperate build directory
```bash
mkdir build
cd build
```
Build it
```bash
cmake ..
make
```
Install the library (option 1, preferred). Make sure you read the prompts that come up regarding expluding certain files.
```bash
sudo checkinstall --pkgnamne libwebsockets-sdev --pkgversion="2.4.1"
```
Note that you can uninstall libwebsockets afterward using apt or dpkg 
if you install libwebsockets with checkinstall, in case something breaks

Install the library (option 2) if you don't have/can't get checkinstall
```bash
sudo make install
```
Update shared library cache so libwebsockets gets linked when we run our bot.
```bash
sudo ldconfig
```
## Build libdiscord

Get a copy of the libdiscord source from github.
```bash
git clone https://github.com/dxing97/libdiscord.git
```
Make a build directory
```bash
cd libdiscord
mkdir build && cd build
```

Build the library and example bots
```bash
cmake ..
make
```

Testing the ayylmao bot:
```bash
./example-ayylmao-bot -t YOUR_BOT_TOKEN
```