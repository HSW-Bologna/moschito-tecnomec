## Requirements

- [SCons](https://scons.org/pages/download.html)
- [emsdk](https://emscripten.org/docs/getting_started/downloads.html)
- build-essential

Note: SDL2 is not explicitly required since Emscripten is gonna use its own port of SDL2. More info: [Emscripten Ports](https://emscripten.org/docs/compiling/Building-Projects.html?highlight=sdl2#emscripten-ports).

## Building

Always wrap `scons` with `emscons` provided by emsdk. 

```bash
emscons scons
```

Use the `--release` option (`emscons scons --release`) to build for a production environment. This will include optimization flags, compact all the web application in a single `app.html` file and generate the compressed `app.html.gz` file.

The counterpart (not using the `--release` option) will just include debug flags useful for debugging the application (see *Debugging* at the end).

### Cleaning

As usual, it is possible to clean up with SCons default `-c` option (`emscons scons -c`). This will remove the application object files and dist files (`app.html`, etc.). Lib object files are not cleaned by choice: the only way to remove them is through a hard clean (or removing them manually from inside the build directory).

For a hard clean do:

```bash
rm build -r # application objects + lib objects 
rm dist -rf # dist files
rm .sconsign.dblite # scons file signature database
```

For a really hard clean also do:

```bash
emcc --clear-ports # clears the local copies of emscripten ports (e.g. SDL2)
emcc --clear-cache # clears the cache of compiled emscripten system libraries 
```

## Execution

Use [`emrun`](https://emscripten.org/docs/compiling/Running-html-files-with-emrun.html) provided by emsdk. This will create a server that hosts the web app.

```bash
emrun dist/app.html
```

## ws_server.py

This is a very simple websocket server that can be used to test the communication with the websocket client of the web app. Requirements: [websockets](https://pypi.org/project/websockets). Execution: `python ws_server.py`.

Currently what the websocket server does at every received message is: wait 3 seconds and send back the message with the last bit toggled (for instance, a boolean is negated or an integer is incremented if even or decremented if odd).

## Debugging

Debug on Chrome / Chromium with Chrome DevTools.

1. Install the following extension: [goo.gle/wasm-debugging-extension](https://goo.gle/wasm-debugging-extension).
2. Go to *Chrome DevTools* > *Settings* (gear icon on the top right corner) > *Experiments* and enable "WebAssembly Debugging: Enable DWARF support".
3. Debug functions are available in the *Sources* panel.

More info: [Debugging WebAssembly with modern tools](https://developer.chrome.com/blog/wasm-debugging-2020/)
