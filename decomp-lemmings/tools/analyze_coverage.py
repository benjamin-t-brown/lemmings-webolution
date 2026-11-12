"""Turn coverage traces into a prioritised recovery work list.

The classification in `tools/classify_inventory.py` gives an upper bound on
work: every recoverable function, whether or not the game ever runs it. This
script narrows that to what a playable build actually needs, by intersecting
the traces produced by `tools/drive_hook_session.py --coverage`.

    python tools/analyze_coverage.py build/coverage/frontend.txt \
        build/coverage/level_mm_dig_000.txt
    python tools/analyze_coverage.py ... --write docs/benchmarks/coverage.json \
        --worklist docs/benchmarks/worklist.txt
"""

import argparse
import json
import re
from pathlib import Path

REPO = Path(__file__).resolve().parents[1]
CLASSIFICATION = REPO / "docs" / "benchmarks" / "function_classification.json"
LEDGER = REPO / "docs" / "FUNCTIONS.md"


def read_trace(path):
    """Ordered list of entry addresses, as first executed."""
    order = []
    seen = set()
    for line in Path(path).read_text(errors="replace").splitlines():
        if not line or line.startswith("#"):
            continue
        parts = line.split()
        if len(parts) != 2:
            continue
        try:
            address = int(parts[1], 16)
        except ValueError:
            continue
        if address not in seen:
            seen.add(address)
            order.append(address)
    return order


def ledger_addresses():
    found = set()
    if not LEDGER.exists():
        return found
    for line in LEDGER.read_text(errors="replace").splitlines():
        if not line.startswith("|"):
            continue
        for match in re.finditer(r"0x([0-9A-Fa-f]{6,8})", line.split("|")[1]):
            found.add(int(match.group(1), 16))
    return found


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("frontend", type=Path, help="frontend-only trace")
    parser.add_argument("level", type=Path, help="trace that reached gameplay")
    parser.add_argument("--write", type=Path)
    parser.add_argument("--worklist", type=Path,
                        help="executed but unrecovered functions, in execution order")
    args = parser.parse_args()

    classification = json.loads(CLASSIFICATION.read_text())
    recoverable = {f["address"] for f in classification["functions"]
                   if f["kind"] in ("named", "unnamed")}
    target_total = classification["target_denominator"]

    frontend = read_trace(args.frontend)
    level = read_trace(args.level)
    frontend_set = set(frontend)
    level_set = set(level)

    executed = frontend_set | level_set
    gameplay_only = level_set - frontend_set
    frontend_only = frontend_set - level_set
    ledger = ledger_addresses()
    recovered_executed = executed & ledger

    print(f"recoverable functions (upper bound):      {target_total}")
    print(f"executed at all across both traces:       {len(executed)}"
          f"  ({100.0 * len(executed) / target_total:.1f}%)")
    print(f"never executed:                           {target_total - len(executed)}"
          f"  ({100.0 * (target_total - len(executed)) / target_total:.1f}%)")
    print()
    print(f"  frontend trace:                         {len(frontend_set)}")
    print(f"  level trace:                            {len(level_set)}")
    print(f"  reached only once in a level:           {len(gameplay_only)}")
    print(f"  reached only in the frontend trace:     {len(frontend_only)}")
    print()
    print(f"ledger entries:                           {len(ledger & recoverable)}")
    print(f"  of those, executed:                     {len(recovered_executed)}")
    print(f"coverage of the executed set:             "
          f"{100.0 * len(recovered_executed) / len(executed):.2f}%")
    print(f"remaining executed work:                  "
          f"{len(executed - ledger)}")

    outside = executed - recoverable
    if outside:
        print()
        print(f"note: {len(outside)} executed addresses are not in the "
              f"recoverable set (instrumented but reclassified)")

    # Execution order is a sensible porting order: callers before callees for
    # anything reached top-down, and startup before gameplay.
    ordered = [a for a in (frontend + [a for a in level if a not in frontend_set])
               if a not in ledger]

    if args.worklist:
        args.worklist.parent.mkdir(parents=True, exist_ok=True)
        lines = [
            "# executed but not in the recovery ledger, in first-execution order",
            "# source: %s + %s" % (args.frontend.name, args.level.name),
            "# phase: F = seen in the frontend trace, G = first seen in a level",
        ]
        for address in ordered:
            phase = "F" if address in frontend_set else "G"
            lines.append("%s %08x" % (phase, address))
        args.worklist.write_text("\n".join(lines) + "\n")
        print(f"\nwrote {len(ordered)} entries to {args.worklist}")

    if args.write:
        args.write.parent.mkdir(parents=True, exist_ok=True)
        args.write.write_text(json.dumps({
            "reference_sha256": classification["reference_sha256"],
            "recoverable_upper_bound": target_total,
            "executed": len(executed),
            "frontend": len(frontend_set),
            "level": len(level_set),
            "gameplay_only": len(gameplay_only),
            "never_executed": target_total - len(executed),
            "ledger": len(ledger & recoverable),
            "ledger_executed": len(recovered_executed),
            "remaining_executed": len(executed - ledger),
            "executed_addresses": ["%08x" % a for a in sorted(executed)],
        }, indent=1) + "\n")
        print(f"wrote {args.write}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
