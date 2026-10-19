"""Prove isolated SpriteView regressions fail and restore the candidate exactly.

Run from PowerShell with normal Python; builds/tests use Invoke-Ucrt64 because
test binaries depend on UCRT64 DLLs.
"""
from __future__ import annotations

import hashlib
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "src/gfx/SpriteView.cpp"
OUTPUT = ROOT / "build/tmp/a3-sprite-view/mutations.json"
INVOKE = ROOT / "scripts/Invoke-Ucrt64.ps1"


def run(command: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run(
        ["powershell", "-NoProfile", "-ExecutionPolicy", "Bypass", "-File", str(INVOKE), command],
        cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=180,
    )


def main() -> int:
    original = SOURCE.read_bytes()
    source = original.decode("utf-8")
    edits = [
        ("legacy_constant_view_0", source.replace(
            "  if (count < 2)\n", "  if (count > 0) return {0, priorMirror};\n  if (count < 2)\n", 1)),
        ("singleton_branch", source.replace(
            "return {0, priorMirror};", "return {1, priorMirror};", 1)),
        ("five_view_diagonal_tie", source.replace(
            "index = -x <= -y ? 2 : 3;", "index = -x < -y ? 2 : 3;", 1)),
        ("eight_view_diagonal_tie", source.replace(
            "index = x <= y ? 6 : 7;", "index = x < y ? 6 : 7;", 1)),
        ("producer_normalization", source.replace(
            "if (length != 0.0) {", "if (length < 0.0) {", 1)),
        ("producer_xz_basis", source.replace(
            "cosine * basis.x - sine * basis.z,", "cosine * basis.z - sine * basis.x,", 1)),
        ("producer_native_pi", source.replace(
            "3.1415927410125732", "3.14159265358979323846", 1)),
        ("producer_negative_wrap", source.replace(
            "score += 2.0;", "score += 3.0;", 1)),
        ("producer_positive_wrap", source.replace(
            "score -= 2.0;", "score -= 3.0;", 1)),
    ]
    assert all(changed != source for _, changed in edits)
    rows = []
    try:
        for name, changed in edits:
            SOURCE.write_text(changed, encoding="utf-8", newline="")
            if SOURCE.read_text(encoding="utf-8") != changed:
                raise RuntimeError(f"write verification failed: {name}")
            build = run("cmake --build --preset dev-debug --target test_spriteview")
            if build.returncode:
                raise RuntimeError(build.stdout)
            test = run("ctest --preset dev-debug -R spriteview")
            rows.append({"mutation": name, "test_exit": test.returncode,
                         "expected_failure": True, "test_output": test.stdout[-3500:]})
            if test.returncode == 0:
                raise RuntimeError(f"mutation survived: {name}")
            SOURCE.write_bytes(original)
            if SOURCE.read_bytes() != original:
                raise RuntimeError(f"restore verification failed: {name}")
    finally:
        if SOURCE.read_bytes() != original:
            SOURCE.write_bytes(original)
    build = run("cmake --build --preset dev-debug --target test_spriteview")
    test = run("ctest --preset dev-debug -R spriteview")
    result = {
        "source_sha256": hashlib.sha256(original).hexdigest(),
        "source_restored_exactly": SOURCE.read_bytes() == original,
        "mutations": rows,
        "restore_build_exit": build.returncode,
        "restore_test_exit": test.returncode,
    }
    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(result, indent=2))
    return 0 if build.returncode == 0 and test.returncode == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
