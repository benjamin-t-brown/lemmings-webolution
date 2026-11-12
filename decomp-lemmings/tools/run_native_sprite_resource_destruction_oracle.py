"""Compare original nested sprite cleanup with only the free boundary adapted."""
import argparse
import hashlib
from pathlib import Path
import struct
import subprocess
import sys
from verify_reference import EXE_SHA256, FUNCTIONS, pe_file_offset

PINS = (
    (0x00484580, 542080, 93, "9118589058a5c8ce5106940debc9c9bfe48515c7979a8a4097d8ce63f141b4a9"),
    (0x004847A0, 542624, 93, "5f6fe25314c54725b71653040ac2e164d38fded0931768a83f97d2cd76c6fc1e"),
    (0x00484990, 543120, 115, "9009539c45189561d495b422d8d9045dd6f4b22a0a5f66702cb9a502273f5cbf"),
    (0x00484470, 541808, 75, "7ecfbef588c24f0b0530bef968ad9289ae5561c554818a12a1ce5040369b41ed"),
    (0x0041A4B0, 107696, 11, "012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5"),
    (0x00484500, 541952, 19, "758ba2e9bd1e02647438bd10c15567ba9d6a86806423ceb4ffc77e9f637a941f"),
    (0x00484520, 541984, 93, "303f0a147c1187a4b9c509e2d070ee92093fd67b07baa05435351fe4fa2fea5a"),
    (0x00484880, 542848, 112, "e104c3c1e6a1af574e74a76aec5260fa415e2f0a3c87d7596fa3afee66b82268"),
    (0x004848F0, 542960, 19, "758ba2e9bd1e02647438bd10c15567ba9d6a86806423ceb4ffc77e9f637a941f"),
    (0x00484910, 542992, 19, "758ba2e9bd1e02647438bd10c15567ba9d6a86806423ceb4ffc77e9f637a941f"),
    (0x00484930, 543024, 93, "da3e5815db16f0b9f2cff77d0a5c19a9b3b748364bdd40140cfe288e992dff3e"),
    (0x00484800, 542720, 112, "a89141090fbe8032d6cf25c47c2f3a3b45bd51bf2b715a8cdde4427a5fd9b34c"),
    (0x00484870, 542832, 11, "012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5"),
    (0x00484A50, 543312, 112, "117ca35fbef076a3e3b950a98221b52fa4e7bd57e2c99d78de8638d11310e5ab"),
    (0x0047C5D3, 509395, 99, "8f6a679ca2fc46b8fdec486fff2908b7de657ea27357e373de23421f203bbf0e"),
    (0x0047C800, 509952, 11, "012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5"),
    (0x00484AC0, 543424, 20, "a068869c343013a89039a5c55be7cdb6c83ff8952b8a0522ce59771b0170b434"),
    (0x00484C70, 543856, 20, "a068869c343013a89039a5c55be7cdb6c83ff8952b8a0522ce59771b0170b434"),
    (0x00484C90, 543888, 20, "a068869c343013a89039a5c55be7cdb6c83ff8952b8a0522ce59771b0170b434"),
    (0x00484CB0, 543920, 20, "a068869c343013a89039a5c55be7cdb6c83ff8952b8a0522ce59771b0170b434"),
    (0x0049C655, 640597, 104, "e7a5b601ea8f3f9000f5f6c522c22bfc271fe8c8645a1ea1a270ef4b5a7ae5f7"),
    (0x0049C6BD, 640701, 24, "2c7fc6efbbb1db82eb61aeff1e738b410fed12f9b658526a8357e8fde26cce90"),
    (0x004850D0, 544976, 75, "6955cf9d156ac7052b6a2aab2aa525b932ac84a51c5a23c71e8d91cd509c8f17"),
    (0x00485250, 545360, 19, "342c81fd54c806d61db72994c542d667431ef9f0ef6f51d67f3d41f82f9718a1"),
    (0x004852B0, 545456, 19, "2ea60ef448944579c91dba652fa31ba4f6eec613f6fae3546dde439a83dc67ec"),
)
ADDRESSES = tuple(row[0] for row in PINS)
CONSTANTS = ()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--exe', type=Path, required=True)
    parser.add_argument('--program', type=Path, required=True)
    args = parser.parse_args()
    data = args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXE_SHA256: parser.error('reference digest mismatch')
    pins = {row[0]: row for row in (*FUNCTIONS, *PINS)}
    blocks = []
    for address in ADDRESSES:
        _, offset, size, digest = pins[address]
        if offset != pe_file_offset(data, address, size): parser.error('PE mapping mismatch')
        body = data[offset:offset + size]
        if hashlib.sha256(body).hexdigest() != digest: parser.error('code digest mismatch')
        blocks.append(struct.pack('<II', address, size) + body)
    for address, expected in CONSTANTS:
        offset = pe_file_offset(data, address, len(expected))
        body = data[offset:offset + len(expected)]
        if body != expected: parser.error('constant mismatch')
        blocks.append(struct.pack('<II', address, len(body)) + body)
    payload = struct.pack('<I', len(blocks)) + b''.join(blocks)
    for _ in range(8):
        result = subprocess.run([str(args.program)], input=payload, capture_output=True, timeout=30)
        if result.returncode != 3:
            sys.stdout.buffer.write(result.stdout)
            sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode


if __name__ == '__main__': raise SystemExit(main())
