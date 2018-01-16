# Building libdiscord

libdiscord uses CMake, so building and installing should be fairly straightforward.

## Dependencies
* libwebsockets (minimum v2.4.1, the package available 
with Ubuntu 17.10 is v2.0.3 and doesn't work so you will likely need to compile yourself)
* jansson (tested to work with v2.10, package available for Ubuntu 17.10)
* libcurl (package available for ubuntu, use libcurl4-gnutls-dev)
* gcc, make, cmake, git (compiler and build tools, git to pull a copy of the source files)

Tested to buuld and work on Raspberry Pi 3 running Raspbian (stretch), 
but if you want to install libwebsockets with checkinstall, 
then you will have to compile checkinstall yourself.

### Quick build instructions for libwebsockets

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

Install the library (option 2)
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