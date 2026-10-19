"""Offline audit of A1 camera operands, context matrices, and optional cache/TL data.

This reads the stock PE and retained snapshots only. Binary32-at-each-operation
is reported as a diagnostic model; this tool does not establish the runtime FPU
control word, depth-state behavior, physical clip planes, or pixel acceptance.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import sys
from pathlib import Path
from typing import Callable


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
DEFAULT_STATES = ROOT / "tools/harness/fixtures/camera_a1"
STATE_NAMES = (
    "reference-state.json",
    "reference-rotation-key-left-state.json",
    "reference-rotation-key-right-state.json",
    "reference-rotation-mouse-state.json",
)
AUTHORITY_SHA256 = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
SOURCE_SHA256 = "4e3a52f579c663fc0e97d217d8ea5d780c246f9c6bb33c112534ac6b20c3b630"
SOURCE_COUNT = 897
SELECTED_SOURCE_INDICES = (26, 25, 41, 40)
MATRIX_ADDRESSES = {
    "projection": 0x58D438,
    "inverse_projection": 0x58D498,
    "camera_composition": 0x58D3D8,
}


def sha256(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def f32(value: float) -> float:
    return struct.unpack("<f", struct.pack("<f", value))[0]


def binary64_bytes(values: list[float]) -> bytes:
    return struct.pack("<12d", *values)


class PEImage:
    """Minimal PE32 reader that rejects VAs not backed by section raw bytes."""

    def __init__(self, path: Path):
        self.path = path
        self.data = path.read_bytes()
        self.digest = sha256(self.data)
        if self.digest != AUTHORITY_SHA256:
            raise ValueError(f"authority SHA-256 mismatch: {self.digest}")
        if self.data[:2] != b"MZ":
            raise ValueError("executable has no DOS MZ header")
        self.pe_offset = struct.unpack_from("<I", self.data, 0x3C)[0]
        if self.data[self.pe_offset:self.pe_offset + 4] != b"PE\0\0":
            raise ValueError("executable has no PE signature")
        coff = self.pe_offset + 4
        machine, section_count = struct.unpack_from("<HH", self.data, coff)
        optional_size = struct.unpack_from("<H", self.data, coff + 16)[0]
        optional = coff + 20
        magic = struct.unpack_from("<H", self.data, optional)[0]
        self.image_base = struct.unpack_from("<I", self.data, optional + 28)[0]
        if machine != 0x14C or magic != 0x10B or self.image_base != 0x400000:
            raise ValueError(
                f"unexpected image identity: machine={machine:#x}, optional={magic:#x}, "
                f"image_base={self.image_base:#x}"
            )
        section_table = optional + optional_size
        sections = []
        for index in range(section_count):
            offset = section_table + index * 40
            name = self.data[offset:offset + 8].split(b"\0", 1)[0].decode("ascii", "replace")
            virtual_size, rva, raw_size, raw_pointer = struct.unpack_from("<IIII", self.data, offset + 8)
            if raw_pointer + raw_size > len(self.data):
                raise ValueError(f"section {name} raw extent exceeds file")
            sections.append({
                "name": name,
                "virtual_size": virtual_size,
                "rva": rva,
                "raw_size": raw_size,
                "raw_pointer": raw_pointer,
            })
        self.sections = sections
        self.header = {
            "machine": machine,
            "optional_magic": magic,
            "image_base": self.image_base,
            "section_count": section_count,
            "sections": sections,
        }

    def read_va(self, va: int, size: int) -> tuple[bytes, dict[str, object]]:
        if size < 0:
            raise ValueError("negative read size")
        rva = va - self.image_base
        for section in self.sections:
            start = int(section["rva"])
            raw_size = int(section["raw_size"])
            offset = rva - start
            if 0 <= offset and offset + size <= raw_size:
                file_offset = int(section["raw_pointer"]) + offset
                raw = self.data[file_offset:file_offset + size]
                if len(raw) != size:
                    break
                return raw, {
                    "va": f"0x{va:08x}",
                    "size": size,
                    "section": section["name"],
                    "file_offset": file_offset,
                    "bytes_hex": raw.hex(),
                    "raw_file_backed": True,
                }
        raise ValueError(f"VA 0x{va:08x}+{size:#x} is not fully file-backed by a PE section")


def relative_call_target(image: PEImage, va: int) -> dict[str, object]:
    raw, record = image.read_va(va, 5)
    if raw[0] != 0xE8:
        raise ValueError(f"expected CALL rel32 at 0x{va:08x}; got {raw.hex()}")
    target = va + 5 + struct.unpack_from("<i", raw, 1)[0]
    record["instruction"] = "CALL rel32"
    record["target_va"] = f"0x{target:08x}"
    return record


def inspect_static_inputs(image: PEImage) -> dict[str, object]:
    call = relative_call_target(image, 0x45DA5F)
    if call["target_va"] != "0x00468e22":
        raise ValueError(f"camera setup call target changed: {call['target_va']}")

    def pushed_binary64(high_va: int, low_va: int, label: str) -> dict[str, object]:
        high, high_record = image.read_va(high_va, 5)
        low, low_record = image.read_va(low_va, 5)
        if high[0] != 0x68 or low[0] != 0x68:
            raise ValueError(f"{label}: expected two PUSH imm32 opcodes at known sites")
        # These sites push high then low dwords; x86 stack order yields low/high bytes.
        raw64 = low[1:5] + high[1:5]
        value = struct.unpack("<d", raw64)[0]
        return {
            "value": value,
            "binary64_hex": value.hex(),
            "stack_order_bytes_hex": raw64.hex(),
            "high_push": high_record,
            "low_push": low_record,
        }

    scale = pushed_binary64(0x468FC9, 0x468FCE, "projection scale")
    inverse = pushed_binary64(0x46906E, 0x469073, "inverse projection scale")
    # Retain bounded windows around the caller and builder, including the actual
    # mapped code bytes used above. These are raw evidence, not decompiler text.
    windows = []
    for label, va, size in (
        ("camera_builder_caller", 0x45DA49, 0x20),
        ("camera_builder_entry", 0x468E22, 0x40),
        ("projection_scale_pushes", 0x468FC9, 10),
        ("inverse_scale_pushes", 0x46906E, 10),
        ("camera_affine_transpose_helper", 0x4692D0, 0x50),
        ("affine_multiply_helper", 0x40EB50, 0x50),
    ):
        raw, record = image.read_va(va, size)
        record["label"] = label
        windows.append(record)
    return {
        "camera_builder_call": call,
        "projection_scale_literal": scale,
        "inverse_projection_scale_literal": inverse,
        "raw_static_windows": windows,
        "interpretation_scope": (
            "The call destination and split PUSH immediate bytes are checked directly. "
            "Other windows are supplied as raw, file-backed bytes for independent review."
        ),
    }


def vecmat(vector: list[float], matrix: list[float], rnd: Callable[[float], float]) -> list[float]:
    output = []
    for column in range(3):
        a = rnd(vector[0] * matrix[column])
        b = rnd(vector[1] * matrix[3 + column])
        c = rnd(vector[2] * matrix[6 + column])
        output.append(rnd(rnd(a + b) + c))
    return output


def compose(left: list[float], right: list[float], rnd: Callable[[float], float]) -> list[float]:
    rows = []
    for row in range(3):
        rows.extend(vecmat(left[3 * row:3 * row + 3], right, rnd))
    translated = vecmat(left[9:12], right, rnd)
    return rows + [rnd(value + offset) for value, offset in zip(translated, right[9:12])]


def derive_matrices(camera: bytes, projection_scale: float, inverse_scale: float,
                    rnd: Callable[[float], float]) -> dict[str, list[float]]:
    if len(camera) != 360:
        raise ValueError(f"camera raw block must be 360 bytes, got {len(camera)}")
    affine = list(struct.unpack_from("<12d", camera, 8))
    # The constructor transposes the 3x3 camera basis; the static helper negates
    # translation before transforming it by that basis.
    basis = [affine[3 * column + row] for row in range(3) for column in range(3)]
    view = basis + vecmat([-value for value in affine[9:12]], basis, rnd)
    result: dict[str, list[float]] = {"view": view}
    for name, offset, factor in (
        ("projection", 0x138, projection_scale),
        ("inverse_projection", 0x150, inverse_scale),
    ):
        operands = struct.unpack_from("<3d", camera, offset)
        matrix = [0.0] * 12
        for index, value in enumerate(operands):
            matrix[index * 4] = rnd(value * factor)
        result[name] = matrix
    result["camera_composition"] = compose(view, result["projection"], rnd)
    return result


def matrix_check(predicted: list[float], observed_hex: str) -> dict[str, object]:
    observed = bytes.fromhex(observed_hex)
    if len(observed) != 96:
        raise ValueError(f"observed matrix must be 96 bytes, got {len(observed)}")
    observed_values = list(struct.unpack("<12d", observed))
    predicted_bytes = binary64_bytes(predicted)
    residuals = [actual - expected for actual, expected in zip(predicted, observed_values)]
    return {
        "predicted_binary64_values": predicted,
        "predicted_binary64_hex": predicted_bytes.hex(),
        "observed_binary64_values": observed_values,
        "observed_hex": observed.hex(),
        "residual_predicted_minus_observed": residuals,
        "max_abs_residual": max(abs(value) for value in residuals),
        "exact_96_bytes": predicted_bytes == observed,
    }


def load_state(path: Path) -> tuple[bytes, dict[str, object], dict[str, object]]:
    raw = path.read_bytes()
    data = json.loads(raw)
    camera_hex = data.get("camera", {}).get("raw_hex")
    if not isinstance(camera_hex, str):
        raise ValueError(f"{path}: missing camera.raw_hex")
    camera = bytes.fromhex(camera_hex)
    if len(camera) != 360:
        raise ValueError(f"{path}: expected 360-byte camera block; got {len(camera)}")
    # Guard metadata against accidental/stale disagreement with raw camera bytes.
    metadata = data["camera"]
    for key, offset, count in (("affine_12_doubles", 8, 12),
                               ("projection_vector", 0x138, 3),
                               ("inverse_projection_vector", 0x150, 3)):
        if key in metadata:
            actual = struct.unpack_from(f"<{count}d", camera, offset)
            declared = tuple(float(value) for value in metadata[key])
            if actual != declared:
                raise ValueError(f"{path}: {key} metadata does not match camera raw bytes")
    return raw, data, camera


def audit_states(image: PEImage, states_dir: Path, static: dict[str, object]) -> tuple[list[dict[str, object]], list[float]]:
    scale = float(static["projection_scale_literal"]["value"])
    inverse_scale = float(static["inverse_projection_scale_literal"]["value"])
    reports = []
    initial_combined: list[float] | None = None
    for filename in STATE_NAMES:
        path = states_dir / filename
        raw, state, camera = load_state(path)
        observed_context = state.get("context_transforms_raw")
        if not isinstance(observed_context, dict):
            raise ValueError(f"{path}: missing context_transforms_raw")
        matrices = {}
        for model_name, rnd in (("binary64", float), ("binary32_diagnostic", f32)):
            predicted = derive_matrices(camera, scale, inverse_scale, rnd)
            checks = {}
            for matrix_name, address in MATRIX_ADDRESSES.items():
                key = f"0x{address:x}"
                try:
                    observed_hex = observed_context[key]["hex"]
                except (KeyError, TypeError) as exc:
                    raise ValueError(f"{path}: missing raw matrix {key}") from exc
                checks[matrix_name] = matrix_check(predicted[matrix_name], observed_hex)
            matrices[model_name] = checks
            if filename == STATE_NAMES[0] and model_name == "binary32_diagnostic":
                initial_combined = predicted["camera_composition"]
        reports.append({
            "file": str(path.resolve().relative_to(ROOT)).replace("\\", "/"),
            "sha256": sha256(raw),
            "pid_metadata_only": state.get("pid"),
            "camera_raw_sha256": sha256(camera),
            "camera_raw_hex": camera.hex(),
            "source_operands_read_from_camera_raw": {
                "camera_plus_0x8_affine_12_f64": list(struct.unpack_from("<12d", camera, 8)),
                "camera_plus_0x138_projection_3_f64": list(struct.unpack_from("<3d", camera, 0x138)),
                "camera_plus_0x150_inverse_projection_3_f64": list(struct.unpack_from("<3d", camera, 0x150)),
            },
            "matrix_models": matrices,
        })
    if initial_combined is None:
        raise AssertionError("initial matrix was not derived")
    return reports, initial_combined


def transform_source(source_xyz: tuple[float, float, float], matrix: list[float]) -> list[float]:
    transformed = vecmat(list(source_xyz), matrix, f32)
    return [f32(value + matrix[9 + index]) for index, value in enumerate(transformed)]


def audit_emission(source_path: Path, emission_dir: Path, initial_matrix: list[float],
                   initial_camera: bytes) -> dict[str, object]:
    source = source_path.read_bytes()
    source_digest = sha256(source)
    if len(source) != SOURCE_COUNT * 24 or source_digest != SOURCE_SHA256:
        raise ValueError(f"source-bank identity/size mismatch: sha256={source_digest}, bytes={len(source)}")
    source_xyz = {
        index: struct.unpack_from("<3d", source, index * 24)
        for index in SELECTED_SOURCE_INDICES
    }
    context_path = emission_dir / "renderer-context.bin"
    emitted_path = emission_dir / "emitted.bin"
    camera_path = emission_dir / "camera.bin"
    context = context_path.read_bytes()
    emitted = emitted_path.read_bytes()
    captured_camera = camera_path.read_bytes()
    if len(context) != 1912 or len(emitted) != 128 or len(captured_camera) != 360:
        raise ValueError("emission input has unexpected renderer-context/emitted/camera size")
    center = struct.unpack_from("<2i", context, 0x754)
    bias = struct.unpack_from("<d", context, 0x770)[0]
    points = []
    for corner, index in enumerate(SELECTED_SOURCE_INDICES):
        source_point = source_xyz[index]
        preprojection = transform_source(source_point, initial_matrix)
        inv_z = f32(1.0 / preprojection[2])
        screen_x = f32(center[0] + f32(inv_z * preprojection[0]))
        screen_y = f32(center[1] - f32(inv_z * preprojection[1]))
        submitted_z = f32(preprojection[2] - bias)
        rhw = f32(0.25 / submitted_z)
        predicted_cache = struct.pack("<3d", preprojection[0], preprojection[1], submitted_z)
        cache_path = emission_dir / f"cache-{corner}.bin"
        cache = cache_path.read_bytes()
        if len(cache) != 104:
            raise ValueError(f"{cache_path}: expected 104 bytes, got {len(cache)}")
        predicted_tl = struct.pack("<4f", screen_x, screen_y, submitted_z, rhw)
        observed_tl = emitted[corner * 32:corner * 32 + 16]
        points.append({
            "corner_order": corner,
            "source_index": index,
            "source_xyz_binary64": list(source_point),
            "preprojection_xyz_binary32_diagnostic": preprojection,
            "renderer_context_center_i32": list(center),
            "renderer_context_bias_f64": bias,
            "predicted_cache_xyz_f64_bytes_hex": predicted_cache.hex(),
            "observed_cache_xyz_f64_bytes_hex": cache[:24].hex(),
            "cache_xyz_byte_exact": predicted_cache == cache[:24],
            "cache_xyz_residual_f64": [
                actual - expected
                for actual, expected in zip(struct.unpack("<3d", cache[:24]),
                                            (preprojection[0], preprojection[1], submitted_z))
            ],
            "predicted_TL_XY_SZ_RHW_f32": [screen_x, screen_y, submitted_z, rhw],
            "predicted_TL_f32_bytes_hex": predicted_tl.hex(),
            "observed_TL_f32_bytes_hex": observed_tl.hex(),
            "TL_XY_SZ_RHW_byte_exact": predicted_tl == observed_tl,
            "TL_component_residual_f32": [
                actual - expected
                for actual, expected in zip(struct.unpack("<4f", observed_tl),
                                            (screen_x, screen_y, submitted_z, rhw))
            ],
            "scope": "cross-capture offline comparison; not same-frame identity",
        })
    return {
        "label": "READ/diagnostic only; not pixel-accepted",
        "source_bank": {
            "path": str(source_path.resolve().relative_to(ROOT)).replace("\\", "/"),
            "sha256": source_digest,
            "record_count": SOURCE_COUNT,
            "record_bytes": 24,
        },
        "emission_capture": str(emission_dir.resolve().relative_to(ROOT)).replace("\\", "/"),
        "input_hashes": {
            str(path.name): sha256(path.read_bytes())
            for path in (context_path, emitted_path, camera_path,
                         *(emission_dir / f"cache-{corner}.bin" for corner in range(4)))
        },
        "camera_raw_equal_to_initial_reference_state": captured_camera == initial_camera,
        "points": points,
        "all_four_cache_xyz_byte_exact": all(point["cache_xyz_byte_exact"] for point in points),
        "all_four_TL_XY_SZ_RHW_byte_exact": all(point["TL_XY_SZ_RHW_byte_exact"] for point in points),
    }


def build_report(exe: Path, states_dir: Path, source_vertices: Path | None,
                 emission_dir: Path | None) -> dict[str, object]:
    image = PEImage(exe)
    static = inspect_static_inputs(image)
    cases, initial_matrix = audit_states(image, states_dir, static)
    report: dict[str, object] = {
        "status": "READ/diagnostic only; not pixel-accepted",
        "pixel_accepted": False,
        "runtime_fpu_precision_proven": False,
        "physical_near_far_claim": False,
        "authority": {
            "path": str(exe.resolve()),
            "sha256": image.digest,
            "pe32_i386_image_base_0x400000": True,
            "header_and_section_map": image.header,
        },
        "static_inputs": static,
        "rounding_models": {
            "binary64": "Python binary64 operation sequence, compared as widened f64 matrix bytes",
            "binary32_diagnostic": "round every modeled mul/add to IEEE binary32, widen to f64 for matrix storage",
            "runtime_mechanism": "not established; no runtime x87 control-word evidence",
        },
        "canonical_camera_states": cases,
        "all_four_binary32_diagnostic_matrix_sets_exact": all(
            all(check["exact_96_bytes"] for check in case["matrix_models"]["binary32_diagnostic"].values())
            for case in cases
        ),
        "all_four_binary64_matrix_sets_within_1e-5": all(
            all(check["max_abs_residual"] <= 1e-5 for check in case["matrix_models"]["binary64"].values())
            for case in cases
        ),
        "source_emission_check": None,
        "limits": [
            "Camera states are retained observations; matrix replay is post-exploratory, not blind preregistration.",
            "Binary32 diagnostic agreement does not prove runtime x87 precision/control state.",
            "Optional cache and TL comparison is cross-capture; matching bytes do not prove same-frame identity.",
            "No game process, live interface, depth-state mutation, pixel render, or physical clip-plane inference is used.",
        ],
    }
    if source_vertices is not None:
        source = source_vertices.read_bytes()
        digest = sha256(source)
        if len(source) != SOURCE_COUNT * 24 or digest != SOURCE_SHA256:
            raise ValueError(f"source-bank identity/size mismatch: sha256={digest}, bytes={len(source)}")
        selected = []
        for index in SELECTED_SOURCE_INDICES:
            xyz = struct.unpack_from("<3d", source, index * 24)
            selected.append({
                "source_index": index,
                "source_xyz_binary64": list(xyz),
                "preprojection_xyz_binary32_diagnostic": transform_source(xyz, initial_matrix),
            })
        report["source_vertices"] = {
            "path": str(source_vertices.resolve().relative_to(ROOT)).replace("\\", "/"),
            "sha256": digest,
            "selected_points_from_initial_derived_matrix": selected,
        }
        if emission_dir is not None:
            initial_camera = bytes.fromhex(str(cases[0]["camera_raw_hex"]))
            report["source_emission_check"] = audit_emission(
                source_vertices, emission_dir, initial_matrix, initial_camera
            )
    return report


def under_root(path: Path, label: str) -> Path:
    resolved = path.resolve()
    try:
        resolved.relative_to(ROOT)
    except ValueError as exc:
        raise ValueError(f"{label} must be inside repository: {resolved}") from exc
    return resolved


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    parser.add_argument("--states-dir", type=Path, default=DEFAULT_STATES)
    parser.add_argument("--source-vertices", type=Path)
    parser.add_argument("--emission-dir", type=Path)
    parser.add_argument("--output", type=Path, help="write JSON under build/tmp; otherwise print it")
    args = parser.parse_args(argv)
    if args.emission_dir is not None and args.source_vertices is None:
        parser.error("--emission-dir requires --source-vertices")
    try:
        states_dir = under_root(args.states_dir, "--states-dir")
        source_vertices = under_root(args.source_vertices, "--source-vertices") if args.source_vertices else None
        emission_dir = under_root(args.emission_dir, "--emission-dir") if args.emission_dir else None
        report = build_report(args.exe, states_dir, source_vertices, emission_dir)
        encoded = json.dumps(report, indent=2, allow_nan=False) + "\n"
        if args.output:
            output = args.output.resolve()
            try:
                output.relative_to((ROOT / "build/tmp").resolve())
            except ValueError as exc:
                raise ValueError("--output must be inside build/tmp") from exc
            output.parent.mkdir(parents=True, exist_ok=True)
            output.write_text(encoded, encoding="utf-8")
            summary = {
                "output": str(output.relative_to(ROOT)).replace("\\", "/"),
                "status": report["status"],
                "all_four_binary32_diagnostic_matrix_sets_exact": report["all_four_binary32_diagnostic_matrix_sets_exact"],
                "all_four_binary64_matrix_sets_within_1e-5": report["all_four_binary64_matrix_sets_within_1e-5"],
                "source_emission_check": None if report["source_emission_check"] is None else {
                    "all_four_cache_xyz_byte_exact": report["source_emission_check"]["all_four_cache_xyz_byte_exact"],
                    "all_four_TL_XY_SZ_RHW_byte_exact": report["source_emission_check"]["all_four_TL_XY_SZ_RHW_byte_exact"],
                    "camera_raw_equal_to_initial_reference_state": report["source_emission_check"]["camera_raw_equal_to_initial_reference_state"],
                },
            }
            print(json.dumps(summary, indent=2))
        else:
            print(encoded, end="")
        emission_ok = report["source_emission_check"] is None or (
            report["source_emission_check"]["all_four_cache_xyz_byte_exact"]
            and report["source_emission_check"]["all_four_TL_XY_SZ_RHW_byte_exact"]
            and report["source_emission_check"]["camera_raw_equal_to_initial_reference_state"]
        )
        return 0 if report["all_four_binary32_diagnostic_matrix_sets_exact"] and emission_ok else 2
    except (OSError, ValueError, KeyError, TypeError, struct.error, json.JSONDecodeError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
