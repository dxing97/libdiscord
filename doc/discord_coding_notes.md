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
    
    
connect to gateway:
if(previous session info does not exist)
    send identify
    etc (normal control flow)
else
    send resume
    set resuming to 1
endif

if(websocket rx resumed)
    set resuming to 0

disconnect from gateway:
set resuming to 0
if(session invalidated)
    delete session info 
else(session might be valid)
    keep current session info
    



```

## Rate Limits

### REST API

#### Global rate limit
Unknown to user code

#### Per-route rate limit

### Websocket API