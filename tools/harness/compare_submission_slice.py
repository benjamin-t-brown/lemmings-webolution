#!/usr/bin/env python3
"""No-forwarding comparator for explicitly synthetic submission-slice JSONL."""
import hashlib
import json
import math
import sys
import copy
from pathlib import Path


class SliceError(Exception):
    pass


def record_raw(destination, raw_records):
    """Write supplied raw JSONL records once, byte-for-byte; never normalize/forward."""
    payload = b"".join(bytes(record) for record in raw_records)
    if not payload.endswith(b"\n"):
        raise SliceError("raw JSONL capture must end at a record boundary")
    with open(destination, "xb") as stream:
        stream.write(payload)
    return {"path": str(destination), "byte_count": len(payload),
            "sha256": hashlib.sha256(payload).hexdigest()}


def _json_object(pairs):
    out = {}
    for key, value in pairs:
        if key in out:
            raise SliceError("duplicate JSON key: " + key)
        out[key] = value
    return out


def _reject_constant(value):
    raise SliceError("non-finite JSON number: " + value)


def _is_int(value):
    return type(value) is int


def _is_number(value):
    if type(value) not in (int, float):
        return False
    try:
        return math.isfinite(value)
    except OverflowError:
        return False


def _need(condition, message):
    if not condition:
        raise SliceError(message)


def _validate_draw(event):
    _need(set(event) == {"sequence", "kind", "draw"}, "invalid event fields")
    _need(_is_int(event["sequence"]) and event["sequence"] >= 0, "invalid event sequence")
    _need(event["kind"] == "draw" and isinstance(event["draw"], dict), "unsupported event")
    draw = event["draw"]
    _need(set(draw) == {"topology", "primitive_count", "stride", "fvf", "space", "vertices", "resource", "viewport", "clear"}, "unsupported draw schema")
    _need(draw["topology"] == "triangle_fan", "unsupported topology")
    _need(draw["space"] == "screen-pixel-rhw", "unsupported/unknown coordinate space")
    for key in ("primitive_count", "stride", "fvf"):
        _need(_is_int(draw[key]) and draw[key] > 0, "invalid draw integer: " + key)
    vertices = draw["vertices"]
    _need(isinstance(vertices, list) and len(vertices) >= 3, "triangle fan needs at least three vertices")
    _need(draw["primitive_count"] == len(vertices) - 2, "fan primitive_count must equal vertex_count - 2")
    for vertex in vertices:
        _need(isinstance(vertex, dict) and set(vertex) == {"xyzw", "diffuse", "specular", "uv"}, "invalid vertex fields")
        _need(isinstance(vertex["xyzw"], list) and len(vertex["xyzw"]) == 4 and all(_is_number(x) for x in vertex["xyzw"]), "invalid xyzw")
        _need(_is_int(vertex["diffuse"]) and 0 <= vertex["diffuse"] <= 0xffffffff, "invalid diffuse")
        _need(type(vertex["specular"]) is str and vertex["specular"] == "unknown",
              'unknown-only scope requires specular="unknown"')
        _need(isinstance(vertex["uv"], list) and len(vertex["uv"]) == 2 and all(_is_number(x) for x in vertex["uv"]), "invalid uv")
    resource = draw["resource"]
    _need(isinstance(resource, dict) and set(resource) == {"id", "generation", "content_version"}, "missing resource identity/version")
    _need(isinstance(resource["id"], str) and bool(resource["id"]), "invalid resource id")
    _need(_is_int(resource["generation"]) and resource["generation"] > 0, "invalid resource generation")
    _need(_is_int(resource["content_version"]) and resource["content_version"] > 0, "invalid resource content version")
    viewport = draw["viewport"]
    _need(isinstance(viewport, dict) and set(viewport) == {"x", "y", "width", "height", "min_z", "max_z"}, "invalid viewport schema")
    _need(all(_is_int(viewport[k]) for k in ("x", "y", "width", "height")) and viewport["width"] > 0 and viewport["height"] > 0, "invalid viewport dimensions")
    _need(all(_is_number(viewport[k]) for k in ("min_z", "max_z")), "invalid viewport depth range")
    clear = draw["clear"]
    _need(isinstance(clear, dict) and set(clear) == {"flags", "color", "depth", "stencil"}, "invalid clear schema")
    _need(isinstance(clear["flags"], list) and clear["flags"] and all(type(x) is str and x in ("target", "z", "stencil") for x in clear["flags"]), "invalid clear flags")
    _need(len(set(clear["flags"])) == len(clear["flags"]), "duplicate clear flag")
    _need(_is_int(clear["color"]) and 0 <= clear["color"] <= 0xffffffff, "invalid clear color")
    _need(_is_number(clear["depth"]) and _is_int(clear["stencil"]) and 0 <= clear["stencil"] <= 255, "invalid clear depth/stencil")


def _strict_json(value):
    return json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=False, allow_nan=False)


def load(path):
    raw = Path(path).read_bytes()
    digest = hashlib.sha256(raw).hexdigest()
    try:
        rows = [json.loads(line, object_pairs_hook=_json_object, parse_constant=_reject_constant) for line in raw.splitlines()]
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        raise SliceError("invalid JSONL: " + str(exc))
    if not rows or any(not isinstance(row, dict) for row in rows):
        raise SliceError("expected JSON object per line")
    head, events = rows[0], rows[1:]
    required = {"record", "schema", "capture", "boundary", "provenance", "scope",
                "coverage", "state_history"}
    if (set(head) != required or head["record"] != "header" or head["schema"] != "synthetic-submission-slice/1"
            or not isinstance(head["capture"], str) or not head["capture"]
            or not isinstance(head["boundary"], str) or not head["boundary"]):
        raise SliceError("invalid header/schema")
    if head["provenance"] != "synthetic; not captured from stock executable":
        raise SliceError("fixture provenance must be explicit synthetic")
    if head["scope"] != {"ignored_unknown_vertex_attribute": "specular"}:
        raise SliceError("unsupported comparison scope")
    cov = head["coverage"]
    covkeys = {"begin_sequence", "end_sequence", "seen", "serialized", "dropped", "decode_failed", "filtered"}
    if (not isinstance(cov, dict) or set(cov) != covkeys
            or any(not _is_int(cov[k]) for k in covkeys)):
        raise SliceError("invalid coverage fields")
    seqs = []
    for event in events:
        _need(isinstance(event, dict) and _is_int(event.get("sequence")), "event sequence missing")
        seqs.append(event["sequence"])
    _need(cov["begin_sequence"] >= 0 and cov["end_sequence"] >= 0 and all(cov[k] >= 0 for k in ("seen", "serialized", "dropped", "decode_failed", "filtered")), "negative coverage count")
    complete = (cov["begin_sequence"] == (seqs[0] if seqs else None)
                and cov["end_sequence"] == (seqs[-1] if seqs else None)
                and cov["seen"] == len(events) and cov["serialized"] == len(events)
                and cov["dropped"] == cov["decode_failed"] == cov["filtered"] == 0
                and cov["end_sequence"] - cov["begin_sequence"] + 1 == len(seqs)
                and seqs == [cov["begin_sequence"] + i for i in range(len(seqs))])
    if not complete:
        return {"path": str(path), "raw_sha256": digest, "raw_bytes": raw,
                "complete": False, "reason": "coverage/history incomplete"}
    _need(isinstance(head["state_history"], list) and len(head["state_history"]) == len(events), "one per-draw state checkpoint required")
    for event, state in zip(events, head["state_history"]):
        _validate_draw(event)
        _need(isinstance(state, dict) and set(state) == {"sequence", "call", "value", "return", "effective"}, "invalid per-draw checkpoint")
        _need(_is_int(state["sequence"]) and state["sequence"] == event["sequence"], "checkpoint sequence must match its draw")
        _need(state["call"] == "request" and state["return"] == "success" and state["effective"] == "unknown", "unsupported request/effective state history")
        value = state["value"]
        _need(isinstance(value, dict) and set(value) == {"blend", "texture"}, "missing required named state fields")
        _need(type(value["blend"]) is bool and _is_int(value["texture"]) and value["texture"] >= 0, "invalid required state types")
    # Keep untouched parsed and byte-level raw records. Exclude only declared unknown specular
    # from the separate comparison view; never mutate the retained raw records.
    raw_records = copy.deepcopy(rows)
    comparable = copy.deepcopy(rows)
    for event in comparable[1:]:
        for vertex in event["draw"]["vertices"]:
            vertex.pop("specular")
    return {"path": str(path), "raw_sha256": digest, "raw_bytes": raw,
            "raw_records": raw_records, "complete": True, "comparable": comparable}


def compare(left, right):
    a, b = load(left), load(right)
    result = {"left": {k: v for k, v in a.items() if k not in ("raw_bytes", "raw_records")},
              "right": {k: v for k, v in b.items() if k not in ("raw_bytes", "raw_records")}}
    if not a["complete"] or not b["complete"]:
        result.update(status="indeterminate", reason="; ".join(x["reason"] for x in (a, b) if not x["complete"]))
    else:
        result["status"] = "complete_equal" if _strict_json(a["comparable"]) == _strict_json(b["comparable"]) else "complete_different"
        result["reason"] = "exact scoped JSON comparison; list/event order preserved"
    return result


def main(argv):
    if len(argv) != 3:
        print("usage: compare_submission_slice.py LEFT.jsonl RIGHT.jsonl", file=sys.stderr)
        return 2
    try:
        print(json.dumps(compare(argv[1], argv[2]), indent=2, sort_keys=True))
    except (OSError, SliceError) as exc:
        print("error: " + str(exc), file=sys.stderr)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
