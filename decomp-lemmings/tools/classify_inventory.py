"""Classify the analysed procedure inventory into work and not-work.

docs/PROGRESS.md counts 6,186 eligible functions but concedes that the
recovered / partial / inspected / unknown split has never been audited, and
that compiler and runtime functions are still included. Planning against that
number over-states the remaining work.

This script reads the stored Ghidra inventory and partitions it. The useful
signal is that Ghidra's Function ID database already matched many library
routines by signature, so they carry real library names (`std::`, `ATL::`,
MFC `C...` classes, CRT helpers) rather than `FUN_xxxxxxxx`. Those are not
game code and should never be scheduled for recovery.

    python tools/classify_inventory.py
    python tools/classify_inventory.py --write docs/benchmarks/function_classification.json
"""

import argparse
import json
import re
from collections import Counter
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
INVENTORY = REPO / "build" / "function_progress_inventory_evidence.json"
LEDGER = REPO / "docs" / "FUNCTIONS.md"

# Names Ghidra's FID database assigns to statically linked library code.
LIBRARY_PATTERNS = (
    r"^std::",
    r"^ATL::",
    r"^FID_conflict:",
    r"^_+[A-Za-z]",               # _mainCRTStartup, __security_init_cookie, ...
    r"^C(Ole|Atl|Archive|Object|String|Wnd|Dialog|Doc|View|Frame|Cmd|Data|Exception|File|Mem|Time|Rect|Point|Size|Map|List|Array|Ptr)",
    r"^operator",
    r"^type_info",
    r"^bad_(alloc|cast|exception|typeid)",
    r"^(logic|runtime|length|domain|range|overflow|underflow|out_of_range|invalid_argument)_error",
    r"^exception",
    r"^basic_(string|istream|ostream|streambuf|ios|filebuf|fstream)",
    r"^(malloc|free|calloc|realloc|memcpy|memset|memmove|strlen|strcpy|strcat|strcmp|sprintf|printf|fprintf|sscanf|fopen|fclose|fread|fwrite|fseek|ftell|atoi|atof|qsort|rand|srand|time|abort|exit|terminate|unexpected)$",
)
LIBRARY_RE = re.compile("|".join(LIBRARY_PATTERNS))

UNNAMED_RE = re.compile(r"^(FUN_|SUB_|thunk_FUN_)[0-9a-fA-F]+$")

# Ghidra synthesises these from the MSVC exception tables. They are
# compiler-generated unwind and catch funclets belonging to a parent routine,
# not separately recoverable functions, and they dominate the high end of
# .text (1,294 of them, mostly above 0x4A8000, spaced 9 to 32 bytes apart).
FUNCLET_RE = re.compile(r"^(Unwind|catch)@[0-9a-fA-F]+$")


def load_rows():
    data = json.loads(INVENTORY.read_text())
    return data["records"][0]["normalized_result"]


def ledger_addresses():
    """Exact procedure entries named in the ledger's first address column."""
    if not LEDGER.exists():
        return set()
    found = set()
    for line in LEDGER.read_text(errors="replace").splitlines():
        if not line.startswith("|"):
            continue
        first = line.split("|")[1]
        for match in re.finditer(r"0x([0-9A-Fa-f]{6,8})", first):
            found.add(int(match.group(1), 16))
    return found


def classify():
    rows = load_rows()
    ledger = ledger_addresses()

    buckets = Counter()
    eligible = []
    for row in rows:
        address = row.get("address") or ""
        procedure = row.get("procedure") or {}
        name = row.get("value") or ""

        if procedure.get("external") or address.startswith("EXTERNAL:"):
            buckets["external"] += 1
            continue
        if procedure.get("thunk"):
            buckets["internal_thunk"] += 1
            continue

        try:
            value = int(address, 16)
        except (TypeError, ValueError):
            buckets["unparseable_address"] += 1
            continue

        if FUNCLET_RE.match(name):
            kind = "funclet"
        elif LIBRARY_RE.search(name):
            kind = "library"
        elif UNNAMED_RE.match(name):
            kind = "unnamed"
        else:
            kind = "named"

        eligible.append({"address": value, "name": name, "kind": kind,
                         "in_ledger": value in ledger})
        buckets[kind] += 1

    return eligible, buckets, ledger


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--write", type=Path, help="write the classification as JSON")
    parser.add_argument("--addresses", type=Path,
                        help="write one hex entry address per line, for the "
                             "coverage tracer: recoverable functions only, so "
                             "unwind funclets are never patched")
    args = parser.parse_args()

    eligible, buckets, ledger = classify()
    total_eligible = len(eligible)
    funclets = buckets["funclet"]
    library = buckets["library"]
    in_ledger = sum(1 for e in eligible if e["in_ledger"])
    target = total_eligible - funclets - library

    print(f"excluded up front: {buckets['external']} external, "
          f"{buckets['internal_thunk']} internal thunks")
    print(f"eligible per the PROGRESS.md rules: {total_eligible}")
    print()
    print(f"  unwind/catch funclets:        {funclets:5d}   not functions")
    print(f"  library (Ghidra FID-matched): {library:5d}   not game code")
    print(f"  named:                        {buckets['named']:5d}")
    print(f"  unnamed (FUN_/SUB_):          {buckets['unnamed']:5d}")
    print()
    print(f"recoverable target denominator: {target}")
    print(f"ledger entries matched in the inventory: {in_ledger} of {len(ledger)} parsed")
    if target:
        print(f"coverage against the target denominator: "
              f"{100.0 * in_ledger / target:.2f}%  "
              f"(vs {100.0 * in_ledger / total_eligible:.2f}% against {total_eligible})")

    funclet_addrs = sorted(e["address"] for e in eligible if e["kind"] == "funclet")
    if funclet_addrs:
        above = sum(1 for a in funclet_addrs if a >= 0x4A8000)
        print()
        print(f"funclet address range: {funclet_addrs[0]:#08x} .. {funclet_addrs[-1]:#08x}")
        print(f"  {above} of {len(funclet_addrs)} sit at or above 0x4a8000")

    if args.write:
        args.write.parent.mkdir(parents=True, exist_ok=True)
        args.write.write_text(json.dumps({
            "reference_sha256": "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f",
            "eligible": total_eligible,
            "funclets": funclets,
            "library": library,
            "target_denominator": target,
            "ledger_matched": in_ledger,
            "functions": eligible,
        }, indent=1) + "\n")
        print(f"\nwrote {args.write}")

    if args.addresses:
        wanted = sorted(e["address"] for e in eligible
                        if e["kind"] in ("named", "unnamed"))
        args.addresses.parent.mkdir(parents=True, exist_ok=True)
        args.addresses.write_text(
            "# recoverable procedure entries of "
            "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f\n"
            "# unwind/catch funclets and FID-matched library routines excluded\n"
            + "".join("%08x\n" % a for a in wanted))
        print(f"wrote {len(wanted)} addresses to {args.addresses}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
