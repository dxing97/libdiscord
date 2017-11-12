#!/usr/bin/env bash

LWS_VERSION=2.4.1
ULFIUS_VERSION=2.2

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
                 make cmake gcc libssl-dev libuv-dev libconfig-dev\
                 libulfius-dev

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

git clone https://github.com/warmcat/libwebsockets
cd libwebsockets
    git checkout v${LWS_VERSION}
    mkdir build && cd build
        cmake ..
        make
        sudo checkinstall --pkgname libwebsockets --pkgversion="2.4.1"
        sudo ldconfig
    cd ..
cd ..

#rm -rf libwebsockets




