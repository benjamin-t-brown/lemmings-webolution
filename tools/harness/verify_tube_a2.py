#!/usr/bin/env python3
"""Independent A2 byte/capture audit and pixel mutation/restore verification.

Run with normal Windows Python (Pillow and Capstone), not the UCRT64 PATH.
--render reruns the nine retained views and isolated diagnostic options, with
a default re-render after each mutation. Original fixtures are read-only.
All generated reports and PNGs stay under ignored build/tmp/a2-tube/verify.
This audits a bounded captured-level fixture, not the full game renderer.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import subprocess
import sys
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / "build/tmp/a2-tube/verify"
AUTHORITY = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
ROI = (100, 80, 280, 410)
THRESHOLD = .02
FIX = ROOT / "tools/harness/fixtures/camera_a1"
ZOOM = ROOT / "build/tmp/a1-zoom-elevation"
VIEWS = {
    "initial": (["--case", "initial"], FIX / "original-0.png"),
    "mouse": (["--case", "mouse"], FIX / "original-rotation-mouse-0.png"),
    "right": (["--case", "right"], FIX / "original-rotation-key-right-0.png"),
    "left": (["--case", "left"], FIX / "original-rotation-key-left-0.png"),
    **{n: (["--params", str(ZOOM / f"params-{n}.json")], ZOOM / f"original-{n}.png")
       for n in ("canonical", "zoom1", "elev_up", "elev_down", "elev0_neg")},
}
MUTATIONS = {
    "no-inner": ["--inner-route", "none"],
    "all-inner": ["--inner-route", "all"],
    "all0": ["--outer-route", "all0"],
    "outer-first": ["--order", "outer-first"],
    "swap": ["--bank-swap", "on"],
    "unflipped": ["--tile-rows", "unflipped"],
}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def read_json(path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def raw_reader(path):
    """Parse raw sizes independently; reject BSS rather than reading a later section."""
    data = path.read_bytes()
    assert hashlib.sha256(data).hexdigest() == AUTHORITY
    pe = struct.unpack_from("<I", data, 0x3c)[0]
    assert data[pe:pe+4] == b"PE\0\0"
    coff = pe + 4
    nsections, = struct.unpack_from("<H", data, coff+2)
    optsize, = struct.unpack_from("<H", data, coff+16)
    opt = coff + 20
    base, = struct.unpack_from("<I", data, opt+28)
    sections = []
    for i in range(nsections):
        at = opt + optsize + 40*i
        name = data[at:at+8].split(b"\0")[0].decode("ascii")
        vsize, rva, rawsize, ptr = struct.unpack_from("<4I", data, at+8)
        sections.append((name, base+rva, vsize, rawsize, ptr))

    def read(va, length):
        for name, start, _, rawsize, ptr in sections:
            if start <= va and va+length <= start+rawsize:
                off = ptr+va-start
                return name, off, data[off:off+length]
        raise ValueError(f"not initialized file bytes: {va:#x}+{length:#x}")
    return sections, read


def audit(exe):
    sections, read = raw_reader(exe)
    dis = Cs(CS_ARCH_X86, CS_MODE_32)
    pins = [
        (0x43A43A, "c1e103", "loader argument bucket*8"),
        (0x43A44E, "e868640200", "calls per-bucket loader 0x4608BB"),
        (0x4608C5, "8b45089983e20703c2c1f8039983e20303c2c1f802", "signed /8 then /4 yields bucket/4"),
        (0x460A10, "83e8012b45fc992bc2d1f8", "(bands-1-band)/2, chop"),
        (0x460A1B, "8b4df88d34c1", "base + row*8"),
        (0x460A2D, "89b0c8000000", "stores cell material at +0xC8"),
        (0x43AB3D, "81c14c040000", "INN name field world+0x44C"),
        (0x43AB68, "8d44ca18", "INN descriptor col+row*8+24"),
        (0x43AB9F, "81c150050000", "OUT name field world+0x550"),
        (0x43ABCA, "8d04ca", "OUT descriptor col+row*8"),
        (0x43A6C0, "dd05b0d25800dc2560144b00dd1db0d25800", "subtract context bias before INN"),
        (0x43A727, "8b80c800000083c018", "INN material cell+0xC8 plus 24"),
        (0x43A754, "e8a47b0200", "INN submission 0x4622FD"),
        (0x43A760, "dd05b0d25800dc0560144b00dd1db0d25800", "restore bias before OUT"),
        (0x43A7F4, "e80b7c0200", "right OUT route 0x462404"),
        (0x43A87C, "e8247b0200", "left OUT route 0x4623A5"),
        (0x43A8FA, "e8667b0200", "center OUT route 0x462465"),
        (0x4623D2, "8b09", "left route uses cell+0 mesh"),
        (0x462432, "8b4904", "right route uses cell+4 mesh"),
        (0x4624B4, "0fbf00", "center +0 child signed short"),
        (0x4624C5, "7408", "center +0 requires child < angle"),
        (0x462528, "0fbf10", "center +4 child signed short"),
        (0x462539, "7508", "center +4 requires child >= angle"),
        (0x4626DA, "8b480c8b55088d0491", "child array +0xC, stride 4"),
        (0x41364A, "83c010", "mesh first quad at object+16"),
        (0x4136CA, "0580000000", "quad stride 128"),
        (0x43A8DD, "dd45ccdc0df0074b00", "center threshold localCenter*8"),
        (0x441884, "e8638cffff", "scene calls tube walker"),
    ]
    citations = []
    for va, expected_hex, claim in pins:
        expected = bytes.fromhex(expected_hex)
        section, off, got = read(va, len(expected))
        assert got == expected, (hex(va), got.hex(), expected_hex)
        citations.append(dict(va=hex(va), file_offset=hex(off), section=section,
                              bytes=got.hex(), claim=claim,
                              instructions=[f"{x.address:#x} {x.mnemonic} {x.op_str}"
                                            for x in dis.disasm(got, va)]))
    constants = []
    for va, expected in ((0x4B1460, .0003), (0x4B1468, .7), (0x4B1050, 32.),
                         (0x4B03A8, .5), (0x4B07F0, 8.)):
        sec, off, got = read(va, 8)
        assert struct.unpack("<d", got)[0] == expected
        constants.append(dict(va=hex(va), file_offset=hex(off), section=sec,
                              bytes=got.hex(), binary64=expected))
    for va in (0x4BFF0C, 0x4BFF24):
        sec, off, got = read(va, 24)
        constants.append(dict(va=hex(va), file_offset=hex(off), section=sec,
                              bytes=got.hex(), text=got.split(b"\0")[0].decode("ascii")))
    data_sec = next(s for s in sections if s[0] == ".data")
    assert data_sec[1]+data_sec[3] == 0x4c6000
    capture_dir = ROOT / "build/tmp/a1-cell-fixture/live-capture"
    capture = read_json(capture_dir / "manifest.json")
    assert capture["loaded_level"].lower() == "compiled\\mm_climb_002.lvl"
    errors, old_errors = [], []
    for e in capture["entries"]:
        g, band = e["group"], e["entry"]
        raw = bytes.fromhex(e["record_hex"])
        observed, = struct.unpack_from("<I", raw, 0xc8)
        expected = g//4 + ((6-1-band)//2)*8
        if observed != expected:
            errors.append([g, band, observed, expected])
        if observed != g//4 + (band//2)*8:
            old_errors.append([g, band, observed])
    assert len(capture["entries"]) == 192 and not errors
    arena_dir = ROOT / "build/tmp/a1-live-arena"
    arena_meta = read_json(arena_dir / "arena.json")
    assert sha(arena_dir / "arena.bin") == arena_meta["sha256"]
    level_path = ROOT / "build/tmp/a1-camera/assets/COMPILED/MM_CLIMB_002.LVL"
    level_bytes = level_path.read_bytes()
    resources = []
    for name in (r"Models\bg_sand_medium\sand_1", r"Models\bg_sand_medium\sky",
                 r"Models\bg_sand_medium\hose", "Entrance.a3d", "Exit.a3d"):
        text = name.encode("ascii")
        needle = struct.pack("<I",len(text))+text
        positions, start = [], 0
        while (at := level_bytes.find(needle,start)) >= 0:
            positions.append(hex(at))
            start = at+len(needle)
        assert positions
        resources.append(dict(name=name, counted_string_offsets=positions,
                              length_prefix_hex=needle[:4].hex()))
    return dict(authority_sha256=AUTHORITY, citations=citations, constants=constants,
                data_initialized_end=hex(data_sec[1]+data_sec[3]),
                level=capture["loaded_level"], cells_checked=192,
                flipped_formula_mismatches=errors, unflipped_formula_mismatches=len(old_errors),
                arena_sha256=arena_meta["sha256"],
                level_sha256=sha(level_path), resource_inventory=resources,
                scope="READ of initialized file bytes and retained runtime capture; no new live measurement")


def error(a, b, roi):
    d = ImageChops.difference(a.crop(roi), b.crop(roi))
    histogram = d.histogram()
    return sum((i % 256)*n for i, n in enumerate(histogram))/(255*3*d.width*d.height)


def pixels(render):
    report = {}
    renderer = ROOT / "tools/harness/render_tube_fixture.py"
    renderer_hash = sha(renderer)
    for name, (params, original_path) in VIEWS.items():
        original_hash = sha(original_path)
        original = Image.open(original_path).convert("RGB")
        assert original.size == (640, 480)
        row = {}
        baseline_hash = None
        baseline = None
        for label, opts in [("default", [])] + list(MUTATIONS.items()):
            png = OUT / f"sourced-candidate-{label}-{name}.png"
            if render:
                command = [sys.executable, str(renderer), *params, "--label", f"{label}-{name}",
                           "--output-dir", str(OUT), *opts]
                subprocess.run(command, cwd=ROOT, check=True, capture_output=True, text=True)
            candidate = Image.open(png).convert("RGB")
            mae = error(original, candidate, ROI)
            row[label] = dict(mae=mae, passes=mae <= THRESHOLD, sha256=sha(png))
            if label == "default":
                baseline_hash = sha(png)
                baseline = candidate
            else:
                diff = ImageChops.difference(baseline, candidate)
                row[label]["same_roi_as_default"] = diff.crop(ROI).getbbox() is None
                row[label]["same_frame_as_default"] = diff.getbbox() is None
                if render:
                    restore_label = f"restore-{label}-{name}"
                    subprocess.run([sys.executable, str(renderer), *params, "--label", restore_label,
                                    "--output-dir", str(OUT)], cwd=ROOT, check=True,
                                   capture_output=True, text=True)
                restore_png = OUT / f"sourced-candidate-restore-{label}-{name}.png"
                assert sha(restore_png) == baseline_hash
                row[label]["default_restored_byte_identical"] = True
        if name == "initial":
            regions = {"cavity": (280,80,430,290), "upper_rim": (100,0,540,35)}
        elif name == "zoom1":
            regions = {"inner_wall": (205,80,275,300)}
        else:
            regions = {}
        row["additional_diagnostics"] = {r: dict(roi=roi, mae=error(original, baseline, roi))
                                         for r, roi in regions.items()}
        assert sha(original_path) == original_hash
        report[name] = row
        print(json.dumps(dict(view=name, default=row["default"]["mae"],
                              mutation_failures=[k for k in MUTATIONS if not row[k]["passes"]])), flush=True)
    assert all(row["default"]["passes"] for row in report.values()), "default fails fixed pixel gate"
    assert all(not row["swap"]["passes"] for row in report.values()), "bank swap survived"
    assert all(not report[n]["no-inner"]["passes"] for n in ("left", "zoom1", "elev_up")), "INN mutation survived"
    assert not report["initial"]["unflipped"]["passes"], "old texture-row formula survived"
    assert sha(renderer) == renderer_hash, "renderer changed during verification"
    return dict(roi=ROI, threshold=THRESHOLD, views=report, renderer_sha256=renderer_hash,
                scope="fixed ROI only; additional failed regions stay failed; no full-frame acceptance")


def live_diagnostics(directory):
    """Re-score retained original isolation independently of the live producer."""
    directory = directory.resolve()
    log = read_json(directory / "isolation.json")
    assert log["success"] and log["code_restored"] and log["camera_unchanged"] and log["paused_at_finish"]
    images = {}
    for name, record in log["captures"].items():
        path = directory / f"{name}.png"
        assert sha(path) == record["sha256"]
        images[name] = Image.open(path).convert("RGB")
    for va in (0x43a4ec,0x4622fd,0x4623a5,0x462404,0x462465):
        rows = [r for r in log["writes"] if r["address"] == hex(va)]
        assert len(rows) == 2 and all(r["readback_ok"] for r in rows)
        assert [(r["before"],r["after"]) for r in rows] == [("55","c3"),("c3","55")]
    baseline = images["baseline"]
    isolation = {}
    for name in ("no-walker", "no-cell-routes"):
        changed, restored = images[name], images[f"restored-{name}"]
        isolation[name] = dict(upper_band_mae=error(baseline,changed,(100,0,540,12)),
                               tube_roi_mae=error(baseline,changed,ROI),
                               restored_roi_mae=error(baseline,restored,ROI),
                               restored_upper_mae=error(baseline,restored,(100,0,540,12)),
                               full_frame_restore_difference_bbox=ImageChops.difference(baseline,restored).getbbox())
        assert isolation[name]["upper_band_mae"] == 0 and isolation[name]["tube_roi_mae"] > THRESHOLD
        assert isolation[name]["restored_roi_mae"] == 0 and isolation[name]["restored_upper_mae"] == 0
    difference = ImageChops.difference(baseline, images["no-cell-routes"])
    r, g, b = difference.split()
    mask = ImageChops.lighter(ImageChops.lighter(r,g),b).point(lambda n: 255 if n else 0)
    count = mask.histogram()[255]
    scores = {}
    for label in ("default", *MUTATIONS):
        candidate = Image.open(OUT / f"sourced-candidate-{label}-initial.png").convert("RGB")
        histogram = ImageChops.difference(baseline,candidate).histogram(mask=mask)
        value = sum((i % 256)*n for i,n in enumerate(histogram))/(255*3*count)
        scores[label] = dict(mae=value, passes=value <= THRESHOLD)
    return dict(isolation=isolation, changed_pixel_count=count, changed_pixel_scores=scores,
                scope="additional predeclared diagnostic; failures retained, not full-frame acceptance")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--exe", type=Path, default=Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe"))
    ap.add_argument("--render", action="store_true")
    ap.add_argument("--bytes-only", action="store_true")
    ap.add_argument("--live", type=Path, help="retained probe_tube_a2 capture directory to audit")
    args = ap.parse_args()
    OUT.mkdir(parents=True, exist_ok=True)
    report = {"bytes": audit(args.exe)}
    if not args.bytes_only:
        report["pixels"] = pixels(args.render)
    if args.live:
        report["live"] = live_diagnostics(args.live)
    (OUT / "verification.json").write_text(json.dumps(report, indent=2)+"\n", encoding="utf-8")
    print(json.dumps({"cells": report["bytes"]["cells_checked"],
                      "unflipped_mismatches": report["bytes"]["unflipped_formula_mismatches"],
                      "report": str(OUT / "verification.json")}))


if __name__ == "__main__":
    main()
