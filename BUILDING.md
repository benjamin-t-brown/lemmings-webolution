# Build Lemmings Webolution from source

Run the build commands from the root of the public source checkout. Native GCC
builds need GCC 13 or later with libstdc++ for `std::format` and `std::expected`.
Use CMake 3.28 or later and Ninja. The desktop game needs SDL2 2.0.22 or later,
including its development headers. [GCC 13 library support](https://gcc.gnu.org/gcc-13/changes.html)

## Linux

On Ubuntu 24.04+ or Debian 13+ install the build dependencies.

```sh
sudo apt update
sudo apt install g++ cmake ninja-build pkg-config libsdl2-dev
```

On Fedora, use `sudo dnf install gcc-c++ cmake ninja-build pkgconf-pkg-config sdl2-compat-devel`.
On Arch, use `sudo pacman -S gcc cmake ninja pkgconf sdl2-compat`.

```sh
cmake --preset gcc-release
cmake --build --preset gcc-release
./build/cmake/gcc-release/webolution --game-dir "$PWD/Lemmings Revolution"
```

Put your two boxes in `Lemmings Revolution/` for the example above, or pass the
path to their folder. You can also put the boxes or a CD image (`.iso` or raw
`.bin`) beside `build/cmake/gcc-release/webolution` and run it with no arguments.

## Windows

Open the MSYS2 UCRT64 shell and install the UCRT64 packages.

```sh
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake \
  mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-SDL2
cmake --preset gcc-release
cmake --build --preset gcc-release
./build/cmake/gcc-release/webolution.exe --game-dir "$PWD/Lemmings Revolution"
```

For a standalone folder with the runtime DLLs, run
`cmake --install build/cmake/gcc-release --prefix "$PWD/build/windows-release"`.
Put the boxes or CD image beside the installed `webolution.exe` to launch it
without `--game-dir`.

## Emscripten

With CMake, Ninja and Python 3 installed, set up the Emscripten SDK. This uses
the same SDK version as the browser workflow.

```sh
git clone --depth 1 --branch 5.0.3 https://github.com/emscripten-core/emsdk.git build/emsdk
./build/emsdk/emsdk install 5.0.3
./build/emsdk/emsdk activate 5.0.3
source ./build/emsdk/emsdk_env.sh
cmake --preset emscripten-release
cmake --build --preset emscripten-release
python3 -m http.server 8000 --bind 127.0.0.1 --directory build/cmake/emscripten-release
```

Open [http://127.0.0.1:8000/webolution.html](http://127.0.0.1:8000/webolution.html)
and choose your boxes or CD image on the setup screen. The page may reload once
to enable WebAssembly threads.
