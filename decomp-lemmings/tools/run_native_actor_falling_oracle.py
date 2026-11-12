"""Run pinned original falling routines with explicit external-call adapters."""

import argparse
import hashlib
from pathlib import Path
import subprocess
import sys

from verify_reference import EXE_SHA256, pe_file_offset


# VA, file offset, length, digest. Order is the native runner's input protocol.
PINS = (
    (0x00416004, 90116, 52, "635f366b99cdef3f5e27fc69b03543df8ffc87bca57633fdb55eadcab22c4797"),
    (0x00416038, 90168, 74, "4e79ab47ece9ce6c65dad17bafa033ae914b7f96c7f1c025c1cf62e78558e316"),
    (0x004160BD, 90301, 682, "b69aeeb62541780fa45716d495a80e0dcfed6ec46671eb863d6e9189b2ac616f"),
    (0x00416367, 90983, 204, "94365b118e003a5e9e84111596d2bddb91420f2ceb2f13de6e54331f94ddaf38"),
    (0x0049C5AC, 640428, 39, "d0a2aca68648a0ae9d5cbfae1b276d4a64d4f797fc0c60f28b64096cdfa3148c"),
    (0x0040DF50, 57168, 85, "442923798ef077222d832bef9326e96123682749444ff3de558159101be4d886"),
    (0x0040BAA0, 47776, 20, "188576ebbe5ebafad22bde4fe5971546cb6997b28c6a73d19d1a686aca4612eb"),
    (0x004B0A08, 723464, 24, "1a8debfe4ec9f968546bfe4b71cd34ce2b8e7a62bf3194e968b52747f49dd77b"),
    (0x004B0338, 721720, 8, "af5570f5a1810b7af78caf4bc70a660f0df51e42baf91d4de5b2328de0e83dfc"),
    (0x004B03E8, 721896, 8, "24b1f4ef66b650ff816e519b01742ff1753733d36e1b4c3e3b52743168915b1f"),
    (0x004B03F8, 721912, 8, "3f710ac088db33363087de2b9a657541fe5447821debaa9fe5cbd538eb1a5f29"),
    (0x004B02D0, 721616, 8, "6c3c396ed6b5c36dcae172271f462051b1266b851e92df3deea8ac65478fd712"),
    (0x004BF354, 783188, 56, "5836a972ff3ba0e8bbbceeeb545e1deb36719b01051b1f00ad746422101039ad"),
)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe", required=True, type=Path)
    parser.add_argument("--program", required=True, type=Path)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256:
        parser.error("reference executable SHA-256 mismatch")
    bodies = []
    for address, offset, length, digest in PINS:
        if pe_file_offset(data, address, length) != offset:
            parser.error(f"0x{address:08X} PE file mapping mismatch")
        body = data[offset:offset + length]
        if hashlib.sha256(body).hexdigest() != digest:
            parser.error(f"0x{address:08X} bytes do not match the reference")
        bodies.append(body)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=b"".join(bodies),
                                capture_output=True, check=False, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    sys.stderr.buffer.write(result.stderr)
    return result.returncode


if __name__ == "__main__":
    raise SystemExit(main())
