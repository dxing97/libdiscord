#!/usr/bin/env bash

LWS_VERSION=2.4.2
#ULFIUS_VERSION=2.3.6
## Uber basic installation script
# WIP, always examine the source before arbitrarily running someone else's scripts

#future plans:
# check what version is required for each package
# if a package is already installed, prompt user for options (keep old, install new, something else)
# Have forks of libwebsockets, ulfius, libcurl as submodules in libdiscord and link them
# statically as an option?
#assuming Ubuntu 17.10, debian 9
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
sudo apt install libjansson-dev libcurl4-openssl-dev\
                 git cmake gcc libssl-dev libconfig-dev\
                 zlib1g-dev libssl-dev libuv1-dev checkinstall
#sudo apt install libulfius-dev
#sudo apt install libwebsockets-dev
#
#echo "Installing libulfius"
## Old makefile based compilation
#git clone https://github.com/babelouest/ulfius.git
#cd ulfius
#    git checkout 2.2.4
#    git submodule update --init
#    git checkout ${ULFIUS_VERSION}
#    cd lib/orcania
#        make && sudo checkinstall --pkgname liborcania-sdev --install=yes
#        cd ../..
#    cd lib/yder
#        make && sudo checkinstall --pkgname libyder-sdev --install=yes
#        cd ../..
#    make && sudo checkinstall --pkgname libulfius-sdev --install=yes
#cd ..
#git clone https://github.com/babelouest/ulfius.git
#cd ulfius
#    git checkout $(ULFIUS_VERSION)
#    git submodule update --init
#    mkdir build && cd build
#        cmake ..
#        make
#        sudo checkinstall --pkgname libulfius-dev --pkgversion="$(ULFIUS_VERSION)"
#        sudo ldconfig
#    cd ..
#cd ..

# need to compile for v2.4.2 (ringbuffer APIs)
echo "Installing libwebsockets"
git clone https://github.com/warmcat/libwebsockets
cd libwebsockets
    git checkout v$(LWS_VERSION)
    mkdir build
    cd build
        cmake .. -DCMAKE_BUILD_TYPE=Debug -DLWS_WITH_LIBUV=ON -DLWS_WITH_LATENCY=ON -DLWS_WITH_IPV6=ON -DLWS_WITH_PLUGINS=ON
        make
        sudo checkinstall --pkgname libwebsockets-dev --pkgversion="$(LWS_VERSION)"
        sudo ldconfig
    cd ..
cd ..


## build libdiscord and example bots
git clone https://github.com/dxing97/libdiscord.git
cd libdiscord
    mkdir build && cd build
        cmake ..
        make
        echo "finished installing dependencies and compiled libdiscord and test apps"
    cd ..
cd ..