#!/usr/bin/env python3
"""Small offline textured-triangle camera fixture, not a game renderer.

Input is intentionally explicit: triangle coordinates are world-space and the
camera object carries READ level/yaw/zoom fields. The candidate builder derives
the 3x3 pose and translation; an optional raw matrix oracle can be compared
separately. This avoids guessing PolyList material/topology semantics.
Projection is x/z + centerX, centerY - y/z with the measured scale vector.
UV sampling is nearest-neighbour; that is a fixture choice, not a claim about
the original sampler.
"""
from __future__ import annotations

import argparse
import json
import math
from pathlib import Path

from PIL import Image


def grabbed_mode2_yaw(params: dict) -> float:
    """Bounded unmodified-key/mouse case with an already verified grab gate.

    This excludes cursor-only, edge rotation, modifiers, flip animation and
    other control modes. One step is one update of the traced control path.
    """
    case=params['control_case']
    yaw=float(params['yaw_units'])
    unit=float(params['rotation_unit'])
    gain=float(params['mouse_gain'])
    left=int(params['left_scan']);right=int(params['right_scan'])
    inversion=float(params['inversion'])
    for step in range(int(case['updates'])):
        local=0.0
        scan=case.get('scan_code')
        if scan==left:local-=unit
        elif scan==right:local+=unit
        if step==0:local+=float(case.get('mouse_dx',0))*gain
        yaw+=-local*inversion/unit
        while yaw>256.0:yaw-=256.0
        while yaw<0.0:yaw+=256.0
    return yaw


def candidate_camera(params: dict) -> tuple[list[float], list[float], float, float]:
    """Build the candidate world-to-view transform from READ parameters.

    Matrix order follows the traced 45CC09 -> 412880 call (yaw composed with
    the fixed X basis); runtime context is an independent oracle for this
    candidate and must be checked before an original-image comparison.
    """
    n = int(params["zoom_shift"])
    if not 0 <= n <= 30:
        raise ValueError("zoom_shift outside this fixture's supported range; this is not a measured game limit")
    scale = float(1 << n)
    pi_code = float(params['pose_pi'])
    tau = 2.0 * pi_code  # Explicit stored operand, separate from the lens PI.
    yaw_units=grabbed_mode2_yaw(params) if 'control_case' in params else float(params['yaw_units'])
    yaw_turns = (yaw_units + float(params['yaw_offset'])) / float(params['yaw_units_per_turn'])
    yaw_angle = tau * yaw_turns
    fixed_pitch_turns = float(params['fixed_pitch_turns'])
    pitch_angle = tau * fixed_pitch_turns
    cy, sy = math.cos(yaw_angle), math.sin(yaw_angle)
    cp, sp = math.cos(pitch_angle), math.sin(pitch_angle)

    # Row/column entries as written by FUN_004821B0 in the two constructors.
    yaw = [[cy, 0.0, -sy], [0.0, 1.0, 0.0], [sy, 0.0, cy]]
    pitch = [[1.0, 0.0, 0.0], [0.0, cp, sp], [0.0, -sp, cp]]
    # 4128E0's composition is described in the accompanying evidence ledger;
    # this row-major product is checked against the independent live oracle.
    pose = [[sum(yaw[i][k] * pitch[k][j] for k in range(3)) for j in range(3)]
            for i in range(3)]

    eye_local = [0.0,
                 (float(params["level_eye_height"]) + float(params['height_bias'])) / scale
                 + (float(params['elevation']) if n != 0 else 0.0),
                 -float(params['eye_distance']) / scale]
    # The raw pose builder applies yaw to the local eye vector (not transpose(yaw)).
    eye_world = [sum(yaw[i][j] * eye_local[j] for j in range(3)) for i in range(3)]

    # Physical pose is Y*X. The view basis is its transpose. FUN_0040EB50
    # applies the transpose of its stored 3x3; column-major physicalView bytes
    # therefore correspond to row-major physicalPose entries.
    world_to_view = [pose[i][j] for i in range(3) for j in range(3)]
    # Translation is -physicalView * eyeWorld = -transpose(pose) * eyeWorld.
    view_translation = [-sum(pose[j][i] * eye_world[j] for j in range(3)) for i in range(3)]
    world_to_view.extend(view_translation)

    theta_turns = float(params["angle_degrees"]) / float(params['angle_units_per_turn'])
    lens_pi=float(params['lens_pi'])
    half_angle = lens_pi * theta_turns
    g = 1.0 / (2.0 * math.tan(half_angle))
    width, height = params["viewport"]
    aspect = float(params['aspect'])
    focal_x = (g / aspect) / (1.0 / width)
    focal_y = (g * height / width) / (1.0 / width)
    return world_to_view, eye_world, focal_x, focal_y


def render(spec: dict) -> Image.Image:
    width, height = spec["viewport"]
    cx, cy = spec["center"]
    if "camera" in spec:
        world_to_view, eye_world, focal_x, focal_y = candidate_camera(spec["camera"])
        spec["world_to_view"] = world_to_view
        spec["camera_build_diagnostic"] = {"eye_world": eye_world, "world_to_view": world_to_view,
                                            "focal_x": focal_x, "focal_y": focal_y}
    else:
        focal_x, focal_y = spec["focal_scale"]
    texture = Path(spec["texture"])
    if not texture.is_absolute():
        texture = Path(spec["_fixture_dir"]) / texture
    tex = Image.open(texture).convert("RGBA")
    tw, th = tex.size
    src = tex.load()
    out = Image.new("RGBA", (width, height), tuple(spec.get("background", [0, 0, 0, 255])))
    dst = out.load()

    # Engine affine layout is 12 doubles: 9 basis values in the
    # FUN_0040EB50 access pattern, then translation x/y/z at indices 9..11.
    # Keep that observed layout explicit rather than reshuffling it.
    m = spec["world_to_view"]
    if len(m) != 12:
        raise ValueError("world_to_view must contain 12 doubles")
    vertices = []
    for v in spec["triangle"]:
        wx, wy, wz, u, vv = v
        x = m[0] * wx + m[3] * wy + m[6] * wz + m[9]
        y = m[1] * wx + m[4] * wy + m[7] * wz + m[10]
        z = m[2] * wx + m[5] * wy + m[8] * wz + m[11]
        if z == 0 or not math.isfinite(z):
            raise ValueError("triangle has zero or non-finite depth")
        sx = cx + focal_x * x / z
        sy = cy - focal_y * y / z
        vertices.append((sx, sy, 1.0 / z, u, vv))

    (ax, ay, aq, au, av), (bx, by, bq, bu, bv), (cxv, cyv, cq, cu, cv) = vertices
    area = (bx - ax) * (cyv - ay) - (by - ay) * (cxv - ax)
    if area == 0 or not math.isfinite(area):
        raise ValueError("degenerate projected triangle")
    xmin = max(0, math.floor(min(ax, bx, cxv)))
    xmax = min(width - 1, math.ceil(max(ax, bx, cxv)))
    ymin = max(0, math.floor(min(ay, by, cyv)))
    ymax = min(height - 1, math.ceil(max(ay, by, cyv)))

    for py in range(ymin, ymax + 1):
        for px in range(xmin, xmax + 1):
            x, y = px + 0.5, py + 0.5
            w0 = ((bx - x) * (cyv - y) - (by - y) * (cxv - x)) / area
            w1 = ((cxv - x) * (ay - y) - (cyv - y) * (ax - x)) / area
            w2 = 1.0 - w0 - w1
            if min(w0, w1, w2) < 0:
                continue
            q = w0 * aq + w1 * bq + w2 * cq
            if q <= 0 or not math.isfinite(q):
                continue
            u = (w0 * au * aq + w1 * bu * bq + w2 * cu * cq) / q
            v = (w0 * av * aq + w1 * bv * bq + w2 * cv * cq) / q
            tx = min(tw - 1, max(0, int(u * tw)))
            ty = min(th - 1, max(0, int(v * th)))
            dst[px, py] = src[tx, ty]
    return out


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("fixture", type=Path)
    ap.add_argument("output", type=Path)
    ns = ap.parse_args()
    spec = json.loads(ns.fixture.read_text(encoding="utf-8-sig"))
    spec["_fixture_dir"] = str(ns.fixture.resolve().parent)
    image = render(spec)
    ns.output.parent.mkdir(parents=True, exist_ok=True)
    image.save(ns.output)
    print(json.dumps({"output": str(ns.output), "size": image.size, "mode": image.mode}))


if __name__ == "__main__":
    main()
