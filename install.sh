#!/usr/bin/env bash

LWS_VERSION=2.4.1
#ULFIUS_VERSION=2.2
## libdiscord dependencies installation script
# WIP, always examine the source before arbitrarily running someone else's scripts

#future plans:
# check what version is required for each package
# if a package is already installed, prompt user for options (keep old, install new, something else)

#assuming Ubuntu 17.10
#assuming the current directory is where everything is going to be downloaded
#assuming sudo access

if [ $(id -u) = 0 ]; then
    echo -n "Warning: Running script as root/superuser. Continue? (Y/N): "
    read yesno
#    echo $yesno
    if [ "$yesno" = "Y" ] || [ "$yesno" = "y" ]; then
        echo -n
    else
        exit 1;
    fi
fi

sudo apt update && sudo apt upgrade
sudo apt install checkinstall libmicrohttpd-dev libjansson-dev libcurl4-gnutls-dev\
                 libgnutls28-dev libgcrypt20-dev git \
                 make cmake gcc libssl-dev libconfig-dev\
                 zlib1g-dev libssl-dev libcurl4-gnutls-dev libuv1-dev
#sudo apt install libulfius-dev
#sudo apt install libwebsockets-dev

##libulfius is not included with Debian 9.3 at the present, and should be compiled.
#echo "Installing libulfius"
#git clone https://github.com/babelouest/ulfius.git
#cd ulfius
#    git checkout ${ULFIUS_VERSION}
#    cd lib/orcania
#        make && sudo checkinstall
#        cd ../..
#    cd lib/yder
#        make && sudo checkinstall
#        cd ../..
#    make && sudo checkinstall
#cd ..

# need to compile for v2.4.1
git clone https://github.com/warmcat/libwebsockets
cd libwebsockets
    git checkout v${LWS_VERSION}
    mkdir build && cd build
        cmake .. -DCMAKE_BUILD_TYPE=Debug DLWS_WITH_LIBUV=ON -DLWS_WITH_LATENCY=ON
        make
        sudo checkinstall --pkgname libwebsockets --pkgversion="2.4.1"
        sudo ldconfig
    cd ..
cd ..


## build libdiscord and example bots
git clone https://github.com/dxing97/libdiscord.git
cd libdiscord
    mkdir build && cd build
        cmake ..
        make
#    cd ..
#cd ..





