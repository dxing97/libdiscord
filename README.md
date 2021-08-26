## This repository has been archived 
libdiscord was developed against the v6 API, which has been depreciated by Discord. 
Functionality of libdiscord in the future is therefore not guaranteed.

Developers interested in using C to interact with Discord are encouraged
to look at alternative libraries such as [orca](https://github.com/cee-studio/orca). 

# libdiscord

A Discord library written in C. Not production ready. 

Join the Discord server: [![Discord](https://discordapp.com/api/guilds/339188611234922507/widget.png)](https://discord.gg/BGgcQQh)


The goal of this library currently is:
* be easy to use

## Building
See [building](doc/BUILDING.md)

Libdiscord was tested against Ubuntu 18.04, Raspbian, and macOS Catalina. 
It has been previously tested to work on ArchLinux, FreeBSD, OpenBSD, and Debian.
The library should build and run on systems that libcurl and libwebsockets supports.
Windows has not been tested, but there's nothing preventing libdiscord from being able to run on Windows. 

## Example Bots
See [example_bots](example_bots) for some sample bots.

## [Install scripts](scripts)
Intended for development use only.

## Todo
See [the todo list](TODO.md) for a list of things that will be implemented at some point. 
Major features and fixes in development for the first point release (v0.3) are:
 * Reconnection logic
 * Jansson memory leaks and other show-stopping bugs