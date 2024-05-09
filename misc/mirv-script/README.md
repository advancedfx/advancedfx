Node 22.1.0 is used.

# Installation

1. Clone repo.
2. Run `npm i`
3. Change server options in `server.ts`, `client.ts` and `index.ts`.
4. Transpile mirv file with `npm run compile`. Find `mirv.js` in `dist` folder.
5. Transpile other files with `npm run compile-scripts`.

# Usage

1. Run server with `npm run server`
2. In game execute `mirv_script_load "./dist/mirv.js"`
3. Run client to send messages `node dist/client.js`. See `client.ts` for more info.

Note: client is based on [simple-websockets](https://github.com/osztenkurden/simple-websockets). Regular client won't work, unless you modify `server.ts` to use other implementation.

Available commands:

-   `{"type": "listTypes"}`
-   `{"type": "quit"}`
-   `{"type": "exec", "data": "command"}`
-   `{"type": "getLastView"}`
-   `{"type": "setView", "data": {"x": 1, "y": 2, "z": 3, "rX": 4, "rY": 5, "rZ": 6, "fov": 7}}`. Values are optional. If some not set, current is used.
-   `{"type": "gameEvents", "data": true}`
-   `{"type": "gameEvents", "data": false}`
-   `{"type": "cViewRenderSetupView", "data": true}`
-   `{"type": "cViewRenderSetupView", "data": false}`

# Development notes

The idea is to abstract `mirv.js` and interact with it through websocket messages.

So, we define the behaviour in `mirv.ts`. Then we handle messages in `server.ts` and use `client.ts`.

`mirv` object in `mirv.ts` is inteface from [rust implementation](https://github.com/advancedfx/advancedfx/blob/mirv-script/AfxHookSource2Rs/src/lib.rs). Same for `wsConnection`.
