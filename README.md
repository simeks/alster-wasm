# wasm playground

## Preq.
* https://emscripten.org/
* https://nodejs.org/en

## Build

```
> mkdir build
> emcmake cmake .. -GNinja
```

## Running

### Node

```
> node alster-wasm.js
```

### Browser

```
> python -m http.server
```
Go to `localhost:8000`


