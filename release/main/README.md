# Lemmings Webolution

An unofficial reimplementation of **Lemmings Revolution** (2000) in C++23.
It uses SDL2 and a software 3D renderer. This project is independent of the
owners of Lemmings Revolution and has no affiliation with them.

## Play in your browser

Open the [browser game](https://benjamin-t-brown.github.io/lemmings-webolution/).
The page may reload once to enable WebAssembly threads.

The project ships no original game data. On the setup screen, choose your
installation folder, select its `lemmings.box` and `patch.box` files, or open
a Lemmings Revolution CD image. You can also download the image through the
setup screen or get it from the [Internet Archive item](https://archive.org/details/lemmings-revolution).
The browser stores imported data and saved progress on this device.

Select **Start game**, then start a new game and choose an unlocked level.
Select a skill on the panel or press **1–8**, then click a lemming to assign it.

## Build on Windows

Use an MSYS2 UCRT64 shell with GCC (C++23), CMake 3.28 or newer, Ninja, and SDL2.

```sh
pacman -S mingw-w64-ucrt-x86_64-{gcc,cmake,ninja,SDL2}
cmake --preset gcc-release
cmake --build --preset gcc-release
./build/cmake/gcc-release/webolution.exe --game-dir "C:/Games/Lemmings Revolution"
```

Point `--game-dir` to the folder containing both game archives. Use the
`gcc-debug` preset for a debug build.

## Build for the browser

Install and activate Emscripten SDK 5.0.3. From a shell with its environment
loaded, run these commands from the project root.

```sh
cmake --preset emscripten-release
cmake --build --preset emscripten-release
python -m http.server 8000 --bind 127.0.0.1 --directory build/cmake/emscripten-release
```

Open [the local build](http://127.0.0.1:8000/webolution.html) and add game data
through the setup screen. The page requires HTTPS or localhost.

## License

The [MIT License](LICENSE) covers this project's source code only. The
original game and its data remain the property of their respective owners.
