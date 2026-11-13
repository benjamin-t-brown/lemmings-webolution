"""Native GCC checks; never launch the original or execute original-byte oracles."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]


def repository_path(value):
    path = (ROOT / value).resolve()
    if not path.is_relative_to(ROOT):
        raise RuntimeError(f'Game/reference path must stay inside the repository: {value}')
    return path


def check(command, cwd=ROOT):
    print(subprocess.list2cmdline([str(x) for x in command]), flush=True)
    subprocess.run([str(x) for x in command], cwd=cwd, check=True)


def check_test_inventory(tests, exe):
    if not tests:
        raise RuntimeError('No tests registered; refusing an empty pass')
    for test in tests:
        # This is an ordinary recovered-C++ unit test, despite its name.
        source_array_test = (test['name'] == 'native_int_array'
                             and Path(test['command'][0]).name == 'test_native_int_array.exe')
        if (test['name'].startswith('native_') and not source_array_test) or any(
                str(exe).replace('\\', '/').lower() in str(arg).replace('\\', '/').lower()
                for arg in test.get('command', [])):
            raise RuntimeError(f'Unexpected original-code test: {test["name"]}')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--profile', choices=['main', 'recovery', 'all'], default='all')
    parser.add_argument('--jobs', type=int, default=12)
    parser.add_argument('--test-jobs', type=int, default=4,
                        help='Concurrent CTest processes; 4 was benchmarked 2026-11-12, 1 is the fallback')
    args = parser.parse_args()
    registry = json.loads((ROOT / 'agent/tools.windows.json').read_text())
    tools = registry['tools']
    for name in ('gcc', 'cmake', 'ctest', 'ninja', 'python', 'git'):
        if not Path(tools[name]['path']).is_file():
            raise RuntimeError(f'Missing registered tool: {name}')
    exe = repository_path(registry['reference']['path'])
    actual = hashlib.sha256(exe.read_bytes()).hexdigest()
    if actual != registry['reference']['sha256']:
        raise RuntimeError(f'Reference identity changed: {actual}')
    print(f'Reference SHA-256 {actual} verified by reading bytes only.', flush=True)
    # Absolute Python remains fixed despite the compiler runtime PATH prefix.
    os.environ['PATH'] = registry['runtime_bin'] + os.pathsep + os.environ['PATH']
    os.environ['SDL_VIDEODRIVER'] = 'dummy'
    os.environ['SDL_AUDIODRIVER'] = 'dummy'
    cmake, ctest = tools['cmake']['path'], tools['ctest']['path']
    target = subprocess.check_output([tools['gcc']['path'], '-dumpmachine'], text=True).strip()
    if target != 'x86_64-w64-mingw32':
        raise RuntimeError(f'Refusing unexpected compiler target: {target}')
    profiles = ['main', 'recovery'] if args.profile == 'all' else [args.profile]
    for profile in profiles:
        source = ROOT if profile == 'main' else ROOT / 'decomp-lemmings'
        build = ROOT / 'build/agent-native' / profile
        configure = [cmake, '-S', source, '-B', build, '-G', 'Ninja',
                     '-DCMAKE_BUILD_TYPE=Debug', '-DBUILD_TESTING=ON',
                     f'-DCMAKE_CXX_COMPILER={tools["gcc"]["path"]}',
                     f'-DCMAKE_MAKE_PROGRAM={tools["ninja"]["path"]}',
                     f'-DPython3_EXECUTABLE={tools["python"]["path"]}',
                     f'-DCMAKE_PREFIX_PATH={Path(registry["runtime_bin"]).parent.as_posix()}']
        if profile == 'main':
            # Optimized with assertions kept (no NDEBUG): the software renderer is several
            # times slower unoptimized, which put levelreturn within 3% of its timeout
            # under four test jobs. Measured in build/test-parallel-benchmark-001/.
            configure += ['-DCMAKE_CXX_FLAGS_DEBUG=-O2 -g']
            configure += [f'-DWEBOLUTION_GAME_DIR={repository_path(registry["assets"]["directory"]).as_posix()}']
        else:
            configure += ['-DLEMMINGS_REFERENCE_EXE=',
                          f'-DLEMMINGS_ARCHIVE={repository_path(registry["assets"]["base"]).as_posix()}',
                          f'-DLEMMINGS_PATCH_ARCHIVE={repository_path(registry["assets"]["patch"]).as_posix()}']
        check(configure)
        check([cmake, '--build', build, '--parallel', str(args.jobs)])
        listing = subprocess.check_output([ctest, '--test-dir', str(build),
                                           '--show-only=json-v1'], text=True)
        tests = json.loads(listing)['tests']
        # A source-only profile must not acquire original-byte oracle tests.
        check_test_inventory(tests, exe)
        (build / 'agent-test-inventory.json').write_text(listing, encoding='utf-8')
        check([ctest, '--test-dir', build, '--output-on-failure',
               '--no-tests=error', '--timeout', '360', '--parallel', str(args.test_jobs),
               '--output-junit', build / 'agent-results.xml'])
        print(f'{profile}: {len(tests)} registered tests passed. '
              'This is source verification, not new native fidelity evidence.', flush=True)
    check([tools['git']['path'], 'diff', '--check'])


if __name__ == '__main__':
    try:
        main()
    except (OSError, RuntimeError, subprocess.CalledProcessError) as error:
        print(f'VERIFICATION FAILED: {error}', file=sys.stderr)
        sys.exit(1)
