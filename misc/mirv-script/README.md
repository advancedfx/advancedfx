# About

This is the set of examples how `mirv_script` feature of HLAE can be used. Only CS2 is supported.

Basically, it allows to run javascript scripts in boajs, which is ECMAScript engine. Note [the limitations of boajs.](https://boajs.dev/conformance)

In game you can use `mirv_script_load` to load a script from a file and `mirv_script_exec` to execute inline.

# Installation

1. Clone repo
2. `cd misc/mirv-script`
3. Install dependencies with `npm i`
4. Change server options in `server.ts`, `client.ts` and `index.mts` if needed
5. Transpile mirv files with `npm run build`
6. Transpile node files with `npm run build-scripts`

Note: Node 22.1.0 is used.

# Usage

1. Run server with `npm run server`
2. In game execute `mirv_script_load "./dist/index.mjs"`
3. Adjust paths in `client.ts` test messages
4. Run test messages with `npm run client`

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

# Notes

Structure:

```ts
\---src
    |   client.ts // example client
    |   index.mts // mirv example entry point
    |   server.ts // example server
    |
    +---mirv
    |       mirv.mts // simple wrapper
	|       hooks.mts // example hooks
    |		test-module.mts // example module
	|		test-hook-module.mts // example hook module
    |       utils.mts
    |       ws-connection.mts // WebSocket connection interface
    |       ws-events-handler.mts // Function to handle incoming websocket messages
    |       ws-events.mts // WebSocket events
    |
    \---types
            mirv.d.ts // Description of mirv interface
```

The general idea of this example is to abstract `mirv` object and interact with it through websocket messages.

Global `mirv` object is interface from [rust implementation](https://github.com/advancedfx/advancedfx/blob/main/AfxHookSource2Rs/src/lib.rs). Sort of the same for `wsConnection`. Types can be found in `types/mirv.d.ts`.

Just explore the files to see examples how it works. In most places there are comments with additional information.

P.S. this is just an example/starting point, your implementation can be different. You can also use `mirv_script` without websockets at all.
