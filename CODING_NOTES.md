## URLs
All URLs defined by libdiscord are set at compile time and can be changed in the CMake build file.
URLs will begin with ``https://`` and will not end with ``/``.

## Connection statuses
Connections to Discord can't be characterized simply as "connected" and "disconnected". Connections include:
* the idempotent REST API
* the websocket API
* the voice connection (not implemented yet)

A bot that's not connected to the gateway can still send API requests over the REST API.
