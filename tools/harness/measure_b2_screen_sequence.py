"""B2a bounded OFFLINE preflight. No launcher or target-memory API is present.

This is a partial identity decoder, NOT a completed capture harness. The real
plan deliberately fails before calling its supplied reader: highlight semantics,
the native input path and the complete safe route have not been proved.
"""
from __future__ import annotations

import argparse
import json
import struct
from pathlib import Path

COUNTER = 0x4C416C
ACTIVE = 0x5D59EC
OWN_ID_OFFSET = 0x2C
BOOT_CHROME = 0x4B1E88

# Read findings do not constitute a full live preflight. Never change these
# flags merely to make a mock pass; each missing edge needs original bytes.
MISSING = (
    "semantic current-process five-row highlight accessor",
    "shim-to-native keyboard poll/dispatch and RETURN/arrow semantics",
    "safe confirm/map/level-entry/paused-ESC/return route and loaded-path reads",
)


class PreflightError(RuntimeError):
    pass


def exact_u32(read, address):
    data = read(address, 4)
    if not isinstance(data, bytes) or len(data) != 4:
        raise PreflightError(f"unreadable dword at {address:#x}")
    return struct.unpack("<I", data)[0]


def object_pointer(value):
    # Sanity only, not proof of allocation, vtable identity or object semantics.
    if value < 0x10000 or value >= 0x80000000 or value % 4:
        raise PreflightError(f"invalid object pointer {value:#x}")
    return value


def identity_snapshot(read, expected_vtable):
    """Partial raw snapshot. Address reads are logged by the supplied reader.

    Bracketing detects a changed pointer/counter, not an atomic game snapshot.
    It cannot detect ABA or same-pointer mutation. No scanning or saved heaps.
    """
    counter = exact_u32(read, COUNTER)
    pointer = object_pointer(exact_u32(read, ACTIVE))
    vtable = exact_u32(read, pointer)
    if vtable != expected_vtable:
        raise PreflightError(f"unexpected vtable {vtable:#x}")
    own_id = exact_u32(read, pointer + OWN_ID_OFFSET)
    if own_id >= counter:
        raise PreflightError("own id is not below construction counter")
    if exact_u32(read, ACTIVE) != pointer or exact_u32(read, COUNTER) != counter:
        raise PreflightError("identity changed while sampled")
    return dict(construction_counter=counter, active_pointer=pointer,
                active_vtable=vtable, active_own_id=own_id)


def require_highlight(value, expected):
    """Control-only predicate; there is intentionally no real highlight reader."""
    if type(value) is not int or value not in range(5) or value != expected:
        raise PreflightError("highlight missing, invalid or different from expected")


def preflight(read):
    # Fail before ANY reader/attach/foreground/input operation. Passing identity
    # mocks is insufficient to remove this unconditional real-plan boundary.
    raise PreflightError("FAIL_DO_NOT_START: " + "; ".join(MISSING))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--preflight", action="store_true", required=True)
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    reads = []
    try:
        preflight(lambda *a: reads.append(a))
    except PreflightError as exc:
        result = dict(status="FAIL_DO_NOT_START", reason=str(exc),
                      missing=list(MISSING), runtime_reads=reads,
                      stock_starts=0, input_events=[], capture_implemented=False)
        args.out.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
        print(json.dumps(result, indent=2))
        return 2
    raise AssertionError("incomplete preflight unexpectedly accepted")


if __name__ == "__main__":
    raise SystemExit(main())
