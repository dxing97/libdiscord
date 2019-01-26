# Discord API Coding Notes

## Disconnections and resuming
### Client side
If the client disconnects from the gateway for whatever reason without a receieved close code from the gateway,

```
connect to the gateway
send a op6 resume 
if sucessfully resumed
    gateway replays events
    gateway sends op0 dispatch type RESUMED
else
    gateway sends op9 invalid session
    

```

## Rate Limits

### REST API

#### Global rate limit
Unknown to user code

#### Per-route rate limit

### Websocket API