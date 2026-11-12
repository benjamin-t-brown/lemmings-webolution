"""List or inspect entries in a user-owned Lemmings Revolution .box archive.

The tool reads source assets in place and never writes their bytes to this
repository. The format was observed in the installed lemmings.box and
patch.box archives; this is an archive-index reader, not a game asset decoder.
"""

import argparse
from dataclasses import dataclass
import hashlib
from pathlib import Path
import struct


@dataclass(frozen=True)
class Entry:
    name: str
    offset: int
    size: int


def read_exact(source, count: int) -> bytes:
    data = source.read(count)
    if len(data) != count:
        raise ValueError("truncated archive")
    return data


def read_index(path: Path) -> list[Entry]:
    file_size = path.stat().st_size
    with path.open("rb") as source:
        header = read_exact(source, 14)
        if header[:6] != b"LEMBOX":
            raise ValueError("invalid LEMBOX signature")
        count, name_bytes = struct.unpack_from("<II", header, 6)
        if count > 1_000_000 or name_bytes > file_size - 14:
            raise ValueError("invalid archive index sizes")
        names_data = read_exact(source, name_bytes)
        names: list[str] = []
        cursor = 0
        for _ in range(count):
            if cursor + 4 > len(names_data):
                raise ValueError("truncated name table")
            length = struct.unpack_from("<I", names_data, cursor)[0]
            cursor += 4
            if length > len(names_data) - cursor:
                raise ValueError("invalid entry name length")
            names.append(names_data[cursor:cursor + length].decode("latin-1"))
            cursor += length
        if cursor != name_bytes:
            raise ValueError("unexpected bytes after entry names")
        if struct.unpack("<I", read_exact(source, 4))[0] != count:
            raise ValueError("offset table count mismatch")
        offsets = struct.unpack(f"<{count}I", read_exact(source, 4 * count))
        if struct.unpack("<I", read_exact(source, 4))[0] != count:
            raise ValueError("size table count mismatch")
        sizes = struct.unpack(f"<{count}I", read_exact(source, 4 * count))
        data_start = source.tell()
    entries = [Entry(name, offset, size)
               for name, offset, size in zip(names, offsets, sizes)]
    previous_end = data_start
    for entry in entries:
        if entry.offset < previous_end or entry.offset + entry.size > file_size:
            raise ValueError(f"invalid entry bounds: {entry.name}")
        previous_end = entry.offset + entry.size
    return entries


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("archive", type=Path)
    parser.add_argument("--filter", default="", help="case-insensitive name substring")
    parser.add_argument("--name", help="inspect one exact archive entry")
    parser.add_argument("--head", type=int, default=32,
                        help="header bytes to print with --name (default: 32)")
    parser.add_argument("--level-header", action="store_true",
                        help="print six unnamed 32-bit fields and four flags with --name")
    args = parser.parse_args()
    if args.head < 0 or args.head > 4096:
        parser.error("--head must be between 0 and 4096")
    if args.level_header and args.name is None:
        parser.error("--level-header requires --name")
    try:
        entries = read_index(args.archive)
    except (OSError, ValueError) as exc:
        parser.error(str(exc))
    print(f"{args.archive}: {len(entries)} entries")
    if args.name is None:
        for entry in entries:
            if args.filter.casefold() in entry.name.casefold():
                print(f"{entry.offset:>10} {entry.size:>10} {entry.name}")
        return 0
    matches = [entry for entry in entries if entry.name.casefold() == args.name.casefold()]
    if len(matches) != 1:
        parser.error(f"expected one entry named {args.name!r}; found {len(matches)}")
    entry = matches[0]
    with args.archive.open("rb") as source:
        source.seek(entry.offset)
        digest = hashlib.sha256()
        head = b""
        remaining = entry.size
        while remaining:
            chunk = read_exact(source, min(remaining, 1 << 20))
            if len(head) < args.head:
                head += chunk[:args.head - len(head)]
            digest.update(chunk)
            remaining -= len(chunk)
    print(f"{entry.name}: offset={entry.offset} size={entry.size}")
    print(f"SHA-256: {digest.hexdigest()}")
    print(f"first {len(head)} bytes: {head.hex(' ')}")
    if args.level_header:
        if entry.size < 40:
            parser.error("entry is too short for the observed level header")
        with args.archive.open("rb") as source:
            source.seek(entry.offset)
            header = read_exact(source, 40)
        if header[:12] != bytes.fromhex("080000004c6576656c203233"):
            parser.error("entry does not start with the observed Level 23 marker")
        fields = struct.unpack_from("<6I", header, 12)
        flags = tuple(header[36:40])
        print(f"level fields (meaning unknown): {fields}")
        print(f"level flags: {flags}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
