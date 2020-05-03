# Building/Compiling libdiscord

libdiscord uses the CMake build system, so building and installing should be fairly straightforward and relatively 
portable.

## Prerequisites
Make sure you have CMake and some sort of C toolchain installed (i.e. ``sudo apt install build-essential cmake``).
Recommended packages: ``checkinstall``: for installing libwebsockets from source, 
``libuv1-dev``: for development purposes (may be required in the future for voice connections)

### Dependencies

Direct dependencies: 
* libwebsockets 
    * minimum required version is v2.4.1, anything below that will not work
    * libwebsockets v4 has not been tested, and is not known to work yet
    * Ubuntu/debian package - ``libwebsockets-dev`` (likely severely out of date, Ubuntu 18.04 uses v2.0.3)
* Jansson 
    * version 2.9 or later should work, v2.7 has been confirmed to NOT work (json_pack changed behavior)
    * Ubuntu/debian package - ``libjansson-dev``
* libcurl 
    * any recent version with SSL support
    * Ubuntu/debian package - ``libcurl4-openssl-dev`` or ``libcurl4-gnutls-dev``
* OpenSSL
    * Only needed for ``example-bot-hash``, will be removed as a direct dependency in future update
    * Ubuntu/debian package - ``libssl-dev``

## Quick build instructions for libwebsockets on Ubuntu

Make sure you have the dependencies for libwebsockets installed.

Get a copy of libwebsockets source from github.
```bash
git clone https://github.com/warmcat/libwebsockets 
```
Checkout the latest version (3.2.2 as of writing)
```bash
cd libwebsockets
git checkout v3.2.2
```
Create a seperate build directory
```bash
mkdir build
cd build
```
Build it. The CMake options listed here are optional, but recommended. ``-DCMAKE_BUILD_TYPE=Debug`` is required for 
debugging, ``-DLWS_WITH_LIBUV=ON`` may be required in the future for UDP voice connections. 
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug -DLWS_WITH_LIBUV=ON -DLWS_WITH_LATENCY=ON -DLWS_WITH_IPV6=ON
make -j4
```
(option 1, preferred) Install the library . Make sure you read the prompts that come up regarding expluding certain files.
```bash
sudo checkinstall --pkgname libwebsockets-dev --pkgversion="3.2.2"
```
Note that you can uninstall libwebsockets afterward using apt or dpkg 
if you install libwebsockets with checkinstall, in case something breaks. Depending on your version of checkinstall, 
checkinstall might not install libwebsockets for you and you may need to run ``dpkg`` to install the generated package.

(option 2) Install the library if you don't have/can't get checkinstall
```bash
sudo make install
```
Update shared library cache so libwebsockets gets linked when we run our bot.
```bash
sudo ldconfig
```
# Building libdiscord
Fairly strightforward, get a copy of the libdiscord source from github.
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

Test the ayylmao bot:
```bash
./example-bot-ayylmao -t YOUR_BOT_TOKEN
```

## Testing history
* Tested to NOT work with libwebsockets v4.x.x, use v3.x.x
* Tested to work with Ubuntu 19.10, LWS v3.2.2
* Tested to build and work on Raspberry Pi 3 and Raspberry Pi W running Raspbian (stretch), 
but note that if you want to install libwebsockets with checkinstall on Raspbian, 
then you will have to compile and install ``checkinstall`` from source yourself (or until the checkinstall package is updated). Otherwise, install with ``sudo make install``

* Tested to work on Ubuntu 18.04.4 LTS

* Works on sparc64 Debian 4.13.4-1 with no issues.

* Works on macOS 10.14 "Mojave" after dependencies are properly installed using Homebrew.