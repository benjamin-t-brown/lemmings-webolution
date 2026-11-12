"""Install or remove the hook harness in a user-owned game installation.

The harness replaces the installation's two-export ``patch.dll`` with a proxy
that forwards ``DirectDrawCreate`` and ``DirectDrawEnumerateA`` to the original
library, renamed ``patch_orig.dll``. Nothing else in the installation is
touched, and no game data is copied into this repository.

The install is idempotent and refuses to destroy the original library: the
proxy carries a marker string, so a second install can tell that the file
currently named ``patch.dll`` is already the proxy rather than the original.

    python hooks/install.py --status  "C:\\progs\\Lemmings Revolution"
    python hooks/install.py --install "C:\\progs\\Lemmings Revolution"
    python hooks/install.py --remove  "C:\\progs\\Lemmings Revolution"
"""

import argparse
import hashlib
import json
import shutil
import sys
from pathlib import Path

# Present in the proxy's string table; absent from the shipped library.
PROXY_MARKER = b"lemmings hook harness"

PROXY_NAME = "patch.dll"
ORIGINAL_NAME = "patch_orig.dll"
RECORD_NAME = "patch_orig.sha256.json"
EXECUTABLE_NAME = "Lemmings Revolution.exe"


def digest(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def is_proxy(path: Path) -> bool:
    return PROXY_MARKER in path.read_bytes()


def describe(install_dir: Path) -> int:
    proxy = install_dir / PROXY_NAME
    original = install_dir / ORIGINAL_NAME

    if not proxy.exists():
        print(f"{PROXY_NAME}: missing")
        return 1

    print(f"{PROXY_NAME}: {proxy.stat().st_size} bytes, "
          f"{'harness proxy' if is_proxy(proxy) else 'original library'}")
    print(f"  sha256 {digest(proxy)}")
    if original.exists():
        print(f"{ORIGINAL_NAME}: {original.stat().st_size} bytes, "
              f"{'harness proxy (WRONG)' if is_proxy(original) else 'original library'}")
        print(f"  sha256 {digest(original)}")
    else:
        print(f"{ORIGINAL_NAME}: absent (harness not installed)")

    log = install_dir / "lemmings_hooks.log"
    print(f"log: {log if log.exists() else 'not written yet'}")
    return 0


def install(install_dir: Path, built_proxy: Path) -> int:
    proxy = install_dir / PROXY_NAME
    original = install_dir / ORIGINAL_NAME

    if not built_proxy.exists():
        print(f"built proxy not found: {built_proxy}", file=sys.stderr)
        print("run hooks/build_hooks.sh first", file=sys.stderr)
        return 1
    if not (install_dir / EXECUTABLE_NAME).exists():
        print(f"{install_dir} does not look like a Lemmings Revolution "
              f"installation ({EXECUTABLE_NAME} is missing)", file=sys.stderr)
        return 1
    if not proxy.exists():
        print(f"{proxy} is missing; nothing to proxy", file=sys.stderr)
        return 1

    if original.exists():
        if is_proxy(original):
            print(f"{original} is the harness proxy, not the original library. "
                  f"Restore it from your installation media before installing.",
                  file=sys.stderr)
            return 1
        print(f"keeping existing backup {original}")
    else:
        if is_proxy(proxy):
            print(f"{proxy} is already the harness proxy but {ORIGINAL_NAME} is "
                  f"absent, so the original library cannot be recovered. "
                  f"Restore patch.dll from your installation media first.",
                  file=sys.stderr)
            return 1
        shutil.copy2(proxy, original)
        record = {
            "original_name": PROXY_NAME,
            "backup_name": ORIGINAL_NAME,
            "sha256": digest(original),
            "bytes": original.stat().st_size,
        }
        (install_dir / RECORD_NAME).write_text(json.dumps(record, indent=2) + "\n")
        print(f"backed up original -> {original} (sha256 {record['sha256']})")

    shutil.copy2(built_proxy, proxy)
    print(f"installed harness proxy -> {proxy}")
    print()
    print("Run the game, then read:")
    print(f"  {install_dir / 'lemmings_hooks.log'}")
    print("Set LEMMINGS_HOOK_MODE to shadow (default), verify, or replace.")
    return 0


def remove(install_dir: Path) -> int:
    proxy = install_dir / PROXY_NAME
    original = install_dir / ORIGINAL_NAME

    if not original.exists():
        print(f"{original} is absent; harness does not appear to be installed")
        return 0
    if is_proxy(original):
        print(f"{original} is the harness proxy, refusing to restore it over "
              f"{PROXY_NAME}", file=sys.stderr)
        return 1

    record_path = install_dir / RECORD_NAME
    if record_path.exists():
        expected = json.loads(record_path.read_text()).get("sha256")
        actual = digest(original)
        if expected and expected != actual:
            print(f"backup digest changed (recorded {expected}, found {actual}); "
                  f"refusing to restore", file=sys.stderr)
            return 1

    shutil.copy2(original, proxy)
    original.unlink()
    record_path.unlink(missing_ok=True)
    print(f"restored original -> {proxy}")
    return 0


def main() -> int:
    repo_root = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("install_dir", type=Path,
                        help="the user-owned Lemmings Revolution installation")
    action = parser.add_mutually_exclusive_group(required=True)
    action.add_argument("--install", action="store_true")
    action.add_argument("--remove", action="store_true")
    action.add_argument("--status", action="store_true")
    parser.add_argument("--built-proxy", type=Path,
                        default=repo_root / "build" / "hooks" / PROXY_NAME,
                        help="proxy produced by hooks/build_hooks.sh")
    args = parser.parse_args()

    install_dir = args.install_dir
    if not install_dir.is_dir():
        print(f"not a directory: {install_dir}", file=sys.stderr)
        return 1

    if args.status:
        return describe(install_dir)
    if args.install:
        return install(install_dir, args.built_proxy)
    return remove(install_dir)


if __name__ == "__main__":
    raise SystemExit(main())
