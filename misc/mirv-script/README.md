# Installation

1. Clone repo
2. `cd misc/mirv-script`
3. Install dependencies with `npm i`
4. Change server options in `server.ts`, `client.ts` and `index.ts` if needed
5. Transpile mirv file with `npm run build`. Find `mirv.js` in `dist` folder
6. Transpile other files with `npm run build-scripts`

Note: Node 22.1.0 is used.

# Usage

1. Run server with `npm run server`
2. In game execute `mirv_script_load "./dist/mirv.js"`
3. Run test messages with `npm run client`

Note: In this example server/client are based on [simple-websockets](https://github.com/osztenkurden/simple-websockets).
If you want to connect without using `simple-websockets` client, then send JSON string in following format or change the implementation:

```typescript
{
	"eventName": string,
	"values": [...args: any] // can be empty
}
```

Available websocket events can be found in `./src/mirv/ws-events.ts`.

Also note the connection parameters, for HLAE its `?hlae=1` and for clients it's `?user=1`. Other connections will be rejected.

# Development notes

The general idea of this example is to abstract `mirv.js` and interact with it through websocket messages.

Structure:

```ts
\---src
    |   client.ts // example client
    |   index.ts // mirv example entry point
    |   server.ts // example server
    |
    +---mirv
    |       mirv.ts // MirvJS class abstraction
    |       utils.ts
    |       ws-connection.ts // WebSocket connection interface
    |       ws-events-handler.ts // Function to handle incoming websocket messages
    |       ws-events.ts // WebSocket events
    |
    \---types
            mirv.d.ts // Description of mirv interface
```

Global `mirv` object is interface from [rust implementation](https://github.com/advancedfx/advancedfx/blob/main/AfxHookSource2Rs/src/lib.rs). Sort of the same for `wsConnection`. Types are defined in `types/mirv.d.ts`.

Check mainly `index.ts, mirv.ts, ws-events-handler.ts` to see how it works. In most places there are comments with additional information.

P.S. this is just an example/starting point, your implementation can be different. You can also use `mirv_script` without websockets at all.
