# About

This contains some examples how `mirv_script` feature of HLAE can be used. Only CS2 is supported.

Basically, it allows to run javascript scripts in boajs, which is ECMAScript engine. Note [the limitations of boajs.](https://boajs.dev/conformance)

In game you can use `mirv_script_load` to load a script from a file and `mirv_script_exec` to execute inline.

# Security warning

**It is very unsafe to execute _untrusted_ scripts**.

The scripting is meant to extend HLAE, as such scripts can have a huge degree of control over your PC.

# Installation

1. Clone repo
2. `cd misc/mirv-script`
3. Install dependencies with `npm i`
4. Run `npm run build` to transpile examples.

# Usage

1. Explore source code of examples in `src/examples`
2. Run examples from their respected folder in `dist` after transpiling.
3. Build your own scripts.

# Notes

- **Attention: Use an HLAE AfxHookSource2 Release build only**, a Debug build will crash ( https://github.com/boa-dev/boa/issues/4089 ).
- Currently hooks might conflict with each other (one might overwrite another). You have to handle it yourself with custom script.
- Global `mirv` object is interface from [rust implementation](https://github.com/advancedfx/advancedfx/blob/main/AfxHookSource2Rs/src/lib.rs). Types can be found in `types/mirv.d.ts`.

Structure:
```
|
\---src
    |
    +---examples // Examples demonstrating some use cases.
    |
    +---snippets // Example snippets - currently unsorted and might conflict with each other and the src example (we need to make plugin system script still).
    |
    \---types // Description of mirv interface, etc.
```
