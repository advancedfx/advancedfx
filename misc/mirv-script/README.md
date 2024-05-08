Node 22.1.0 is used.

# Installation

1. Clone repo.
2. Run `npm i`
3. Change server options in `server.ts` and `index.ts`.
4. Transpile mirv file with `npm run compile`. Find `mirv.js` in `dist` folder.
5. Run server with `npm run server`. It will transpile and run server file.

# Usage

1. Use `mirv_script_load "./dist/mirv.js"`
2. Connect to ws server with `ws://localhost:31337/mirv?user=1`
3. Send messages in following format: `{"type": "message", "data": "Hello world!"}`

Available commands:

-   `{"type": "listTypes"}`
-   `{"type": "quit"}`
-   `{"type": "exec", "data": "command"}`
-   `{"type": "getLastView"}`
-   `{"type": "setView", "data": {"x": 1, "y": 2, "z": 3, "rX": 4, "rY": 5, "rZ": 6, "fov": 7}}`
-   `{"type": "gameEvents", "data": "true"}`
-   `{"type": "gameEvents", "data": "false"}`
-   `{"type": "cViewRenderSetupView", "data": "true"}`
-   `{"type": "cViewRenderSetupView", "data": "false"}`

On windows you can use `wscat` to connect and send messages:

```
npm i -g wscat
wscat --connect ws://localhost:31337/mirv?user=1
```
