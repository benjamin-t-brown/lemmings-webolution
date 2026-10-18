"""Read decoded camera-fixture BMP carriers without inferring material opacity.

This checks file headers, not the stock texture loader, external masks, or
which textures the stock renderer submits. It never modifies the inputs.
"""
import argparse
import hashlib
import json
import struct
from pathlib import Path


def inspect(path: Path) -> dict:
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"{path}: not a Windows BMP header")
    offset, dib_size = struct.unpack_from("<II", data, 10)
    if dib_size < 40 or 14 + dib_size > len(data):
        raise ValueError(f"{path}: unsupported or incomplete DIB header")
    width, height, planes, bits, compression = struct.unpack_from("<iiHHI", data, 18)
    if width <= 0 or height == 0 or planes != 1 or not 14 + dib_size <= offset <= len(data):
        raise ValueError(f"{path}: invalid bitmap dimensions or pixel offset")
    ordinary_rgb = bits == 24 and compression == 0
    if ordinary_rgb:
        stride = ((width * bits + 31) // 32) * 4
        if offset + stride * abs(height) > len(data):
            raise ValueError(f"{path}: truncated RGB pixel array")
    return {
        "file": str(path), "sha256": hashlib.sha256(data).hexdigest(),
        "width": width, "stored_height": height, "bits_per_pixel": bits,
        "compression": compression,
        "carrier_has_no_embedded_alpha": True if ordinary_rgb else None,
    }


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("directory", type=Path)
    parser.add_argument("--pattern", default="*.bmp")
    args = parser.parse_args()
    paths = sorted(args.directory.glob(args.pattern))
    if not paths:
        parser.error("no matching decoded BMPs")
    records = [inspect(path) for path in paths]
    print(json.dumps({
        "scope": "Decoded carriers only; no claim about material opacity, external alpha, or stock draw selection",
        "count": len(records), "files": records,
    }, indent=2))


if __name__ == "__main__":
    main()
