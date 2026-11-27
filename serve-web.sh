#!/usr/bin/env bash
set -e

cd -- "$(dirname -- "${BASH_SOURCE[0]}")"

emsdk_version=5.0.3
preset=emscripten-release
build_dir="build/cmake/$preset"

python_check='import sys; assert sys.version_info.major == 3; print(sys.executable)'
if ! python_path=$(python3 -c "$python_check" 2>/dev/null) &&
   ! python_path=$(python -c "$python_check" 2>/dev/null) &&
   ! python_path=$(py -3 -c "$python_check" 2>/dev/null); then
    printf 'Python 3 is required to build and serve Webolution.\n' >&2
    exit 1
fi
export EMSDK_PYTHON="${EMSDK_PYTHON:-$python_path}"

sdk_dir=${EMSDK:-}
if [[ -z "$sdk_dir" && -f "$build_dir/CMakeCache.txt" ]]; then
    # Reuse the SDK that configured this build, even from a fresh shell.
    sdk_dir=$(sed -n 's|^CMAKE_TOOLCHAIN_FILE:[^=]*=\(.*\)/upstream/emscripten/cmake/Modules/Platform/Emscripten\.cmake[[:space:]]*$|\1|p' \
        "$build_dir/CMakeCache.txt")
fi
if [[ -z "$sdk_dir" || ! -f "$sdk_dir/emsdk_env.sh" ]]; then
    sdk_dir=build/emsdk
    if [[ ! -f "$sdk_dir/emsdk_env.sh" ]]; then
        git clone --depth 1 --branch "$emsdk_version" https://github.com/emscripten-core/emsdk.git "$sdk_dir"
    fi
    "$sdk_dir/emsdk" install "$emsdk_version"
    "$sdk_dir/emsdk" activate "$emsdk_version"
fi
source "$sdk_dir/emsdk_env.sh"

cmake --preset "$preset"
cmake --build --preset "$preset"

printf '\nOpen http://127.0.0.1:8000/webolution.html (Ctrl+C to stop).\n'
exec "$python_path" -m http.server 8000 --bind 127.0.0.1 --directory "$build_dir"
