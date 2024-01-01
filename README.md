# wasm playground

Just playing around with some C++/wasm stuff

## Preq.
* https://emscripten.org/
* https://nodejs.org/en

## Build

```
> mkdir build
> emcmake cmake .. -GNinja
```

## Running

### Browser

```
> python -m http.server
```
Go to `localhost:8000/alster-wasm.html`


