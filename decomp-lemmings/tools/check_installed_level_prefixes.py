"""Check the C++ prefix reader against all installed compiled levels."""

import argparse
from collections import Counter
from pathlib import Path
import struct
import subprocess

from inspect_box import read_index
from parse_level_entities import parse_gadgets


def hash_names(names: list[bytes]) -> int:
    value = 14695981039346656037
    for name in names:
        for byte in name.split(b"\x00", 1)[0]:
            value = ((value ^ byte) * 1099511628211) & 0xFFFFFFFFFFFFFFFF
        value = ((value ^ 0xFF) * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return value


def hash_bytes(data: bytes) -> int:
    value = 14695981039346656037
    for byte in data:
        value = ((value ^ byte) * 1099511628211) & 0xFFFFFFFFFFFFFFFF
    return value


def parse_table(data: bytes, cursor: int) -> tuple[list[bytes], int]:
    count = struct.unpack_from("<I", data, cursor)[0]
    cursor += 4
    names = []
    for _ in range(count):
        length = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        if length > 254 or length > len(data) - cursor:
            raise ValueError("invalid installed level name length")
        names.append(data[cursor:cursor + length])
        cursor += length
    return names, cursor


def parse_string(data: bytes, cursor: int) -> tuple[bytes, int]:
    length = struct.unpack_from("<I", data, cursor)[0]
    cursor += 4
    if length > 254 or length > len(data) - cursor:
        raise ValueError("invalid installed object string length")
    return data[cursor:cursor + length], cursor + length


def parse_entities(data: bytes, cursor: int) -> tuple[tuple[int, ...], int]:
    registry, groups = struct.unpack_from('<II', data, cursor)
    cursor += 8
    fields = [registry, groups]
    for _ in range(groups):
        flag, count = struct.unpack_from('<BI', data, cursor)
        cursor += 5
        records = data[cursor:cursor + 44 * count]
        cursor += len(records)
        integers = struct.unpack_from('<I', data, cursor)[0]
        cursor += 4
        values = data[cursor:cursor + 4 * integers]
        cursor += len(values)
        name, cursor = parse_string(data, cursor)
        fields.extend((int(bool(flag)), count, hash_bytes(records),
                       integers, hash_bytes(values), hash_names([name])))
    pairs = struct.unpack_from('<I', data, cursor)[0]
    cursor += 4
    fields.append(pairs)
    for _ in range(pairs):
        flag = data[cursor]
        first, cursor = parse_string(data, cursor + 1)
        second, cursor = parse_string(data, cursor)
        fields.extend((int(bool(flag)), hash_names([first, second])))
    for width in (110, 111):
        count = struct.unpack_from('<I', data, cursor)[0]
        cursor += 4
        values = bytearray(data[cursor:cursor + count * width])
        assert len(values) == count * width
        cursor += len(values)
        for start in range(0, len(values), width):
            for offset in (40, 41, 52, 53, 54, 55, 60, 61):
                values[start + offset] = int(bool(values[start + offset]))
            if width == 111:
                values[start + 110] = int(bool(values[start + 110]))
        fields.extend((count, hash_bytes(values)))
    return tuple(fields), cursor


def parse_class_one(data: bytes, cursor: int) -> tuple[tuple[int, ...], int]:
    first_count = struct.unpack_from("<I", data, cursor)[0]
    cursor += 4
    first_bytes = data[cursor:cursor + 24 * first_count]
    cursor += len(first_bytes)
    second_count = struct.unpack_from("<I", data, cursor)[0]
    cursor += 4
    second_bytes = data[cursor:cursor + 24 * second_count]
    cursor += len(second_bytes)
    reference_count = struct.unpack_from("<I", data, cursor)[0]
    cursor += 4
    fields = [first_count, hash_bytes(first_bytes), second_count,
              hash_bytes(second_bytes), reference_count]
    for _ in range(reference_count):
        name, cursor = parse_string(data, cursor)
        value = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        fields.extend((hash_names([name]), value))
    polygon_count = struct.unpack_from("<I", data, cursor)[0]
    cursor += 4
    fields.append(polygon_count)
    for _ in range(polygon_count):
        allocated_bytes = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        polygon_marker, cursor = parse_string(data, cursor)
        polygon_type, record_count, extra_value = struct.unpack_from(
            "<III", data, cursor)
        cursor += 12
        assert polygon_type == 4
        records = data[cursor:cursor + 96 * record_count]
        cursor += len(records)
        fields.extend((allocated_bytes, int(polygon_marker == b"PolyList 09"),
                       polygon_type, record_count, extra_value,
                       hash_bytes(records)))
    doubles = data[cursor:cursor + 96]
    cursor += len(doubles)
    following_name, cursor = parse_string(data, cursor)
    final_doubles = data[cursor:cursor + 16]
    cursor += len(final_doubles)
    nested_count = struct.unpack_from("<I", data, cursor)[0]
    cursor += 4
    fields.extend((hash_bytes(doubles), hash_names([following_name]),
                   hash_bytes(final_doubles), nested_count))
    for _ in range(nested_count):
        tag = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        assert tag == 1
        nested, cursor = parse_class_one(data, cursor)
        fields.extend((tag, *nested))
    return tuple(fields), cursor


def parse_object_child(data: bytes, cursor: int, depth: int = 0, stats=None) -> tuple[tuple[int, ...], int]:
    assert depth <= 256
    tag = struct.unpack_from("<I", data, cursor)[0]
    if stats is not None:
        stats[f"class_{tag}"] += 1
    cursor += 4
    fields = [tag]
    if tag == 1:
        for _ in range(2):
            count = struct.unpack_from("<I", data, cursor)[0]
            cursor += 4
            triples = data[cursor:cursor + 24 * count]
            assert len(triples) == 24 * count
            cursor += len(triples)
            fields.extend((count, hash_bytes(triples)))
        references = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        fields.append(references)
        for _ in range(references):
            name, cursor = parse_string(data, cursor)
            value = struct.unpack_from("<I", data, cursor)[0]
            cursor += 4
            fields.extend((hash_names([name]), value))
        polygons = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        fields.append(polygons)
        for _ in range(polygons):
            allocated = struct.unpack_from("<I", data, cursor)[0]
            cursor += 4
            marker, cursor = parse_string(data, cursor)
            poly_type, record_count, extra = struct.unpack_from("<III", data, cursor)
            if stats is not None:
                stats[f"polygon_{poly_type}"] += 1
            cursor += 12
            assert poly_type in (4, 5, 6, 7), f"polygon type {poly_type} at {cursor - 12}"
            stride = {4: 96, 5: 160, 6: 116, 7: 212}[poly_type]
            records = data[cursor:cursor + record_count * stride]
            assert len(records) == record_count * stride
            cursor += len(records)
            fields.extend((allocated, int(marker == b"PolyList 09"),
                           poly_type, record_count, extra, hash_bytes(records)))
    elif tag == 2:
        leading = data[cursor:cursor + 24]
        cursor += 24
        first_flag, mode = data[cursor:cursor + 2]
        cursor += 2
        if mode:
            resource_data = data[cursor:cursor + 16]
            cursor += 16
            resource_name, cursor = parse_string(data, cursor)
        else:
            resource_name, cursor = parse_string(data, cursor)
            resource_data = data[cursor:cursor + 32]
            cursor += 32
        final_value = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        fields.extend((hash_bytes(leading), int(bool(first_flag)), int(bool(mode)),
                       hash_names([resource_name]), hash_bytes(resource_data),
                       final_value))
    elif tag == 3:
        fields.append(hash_bytes(data[cursor:cursor + 80]))
        cursor += 80
    else:
        assert tag == 0
    base_doubles = data[cursor:cursor + 96]
    cursor += 96
    base_name, cursor = parse_string(data, cursor)
    final_doubles = data[cursor:cursor + 16]
    cursor += 16
    nested_count = struct.unpack_from("<I", data, cursor)[0]
    cursor += 4
    fields.extend((hash_bytes(base_doubles), hash_names([base_name]),
                   hash_bytes(final_doubles), nested_count))
    for _ in range(nested_count):
        nested, cursor = parse_object_child(data, cursor, depth + 1, stats)
        fields.extend(nested)
    return tuple(fields), cursor


def parse_auxiliary_records(data: bytes, cursor: int, count: int) -> tuple[tuple[int, ...], int]:
    fields = []
    for _ in range(count):
        start = cursor
        leading = struct.unpack_from("<Q", data, cursor)[0]
        cursor += 8
        first_count = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        for _ in range(first_count):
            nested = struct.unpack_from("<I", data, cursor + 4)[0]
            cursor += 8 + nested * 89
        second_count = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4
        for _ in range(second_count):
            nested = struct.unpack_from("<I", data, cursor + 4)[0]
            cursor += 8 + nested * 40 + 4
            for _ in range(2):
                integers = struct.unpack_from("<I", data, cursor)[0]
                cursor += 4 + integers * 4
        third_count = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4 + third_count * 16
        name, cursor = parse_string(data, cursor)
        trailing = struct.unpack_from("<I", data, cursor)[0]
        cursor += 4 + trailing * 4
        assert cursor <= len(data)
        fields.extend((leading, first_count, second_count, third_count,
                       hash_names([name]), trailing, hash_bytes(data[start:cursor])))
    return tuple(fields), cursor


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--probe", required=True, type=Path)
    parser.add_argument("--archive", required=True, type=Path, nargs="+")
    args = parser.parse_args()
    payload = bytearray()
    expected = []
    fourth_total = 0
    fifth_total = 0
    second_tags = Counter()
    entity_stats = Counter()
    geometry_stats = Counter()
    for archive in args.archive:
        with archive.open("rb") as source:
            for entry in read_index(archive):
                if not (entry.name.casefold().startswith("compiled\\") and
                        entry.name.casefold().endswith(".lvl")):
                    continue
                source.seek(entry.offset)
                data = source.read(entry.size)
                if len(data) != entry.size or data[:12] != bytes.fromhex(
                        "080000004c6576656c203233"):
                    parser.error(f"unexpected level data: {archive.name}:{entry.name}")
                first_field = struct.unpack_from("<I", data, 12)[0]
                first, cursor = parse_table(data, 40)
                second, cursor = parse_table(data, cursor)
                third, cursor = parse_table(data, cursor)
                object_count = struct.unpack_from("<I", data, cursor)[0]
                cursor += 4
                head = ()
                if object_count:
                    outer, cursor = parse_string(data, cursor)
                    leading_value = struct.unpack_from("<I", data, cursor)[0]
                    cursor += 4
                    inner, cursor = parse_string(data, cursor)
                    double_bits = struct.unpack_from("<QQQ", data, cursor)
                    cursor += 24
                    marker, cursor = parse_string(data, cursor)
                    class_tag = struct.unpack_from("<I", data, cursor)[0]
                    cursor += 4
                    class_head = (0,)
                    if class_tag == 1:
                        parsed, cursor = parse_class_one(data, cursor)
                        first_flag = data[cursor]
                        list_flag = data[cursor + 1]
                        cursor += 2
                        auxiliary_count = 0
                        if list_flag:
                            auxiliary_count = struct.unpack_from("<I", data, cursor)[0]
                            cursor += 4
                            assert auxiliary_count == 0
                        final_flag = data[cursor]
                        cursor += 1
                        final_name = b""
                        if final_flag:
                            final_name, cursor = parse_string(data, cursor)
                        class_head = (1, *parsed, int(bool(first_flag)),
                                      int(bool(list_flag)), auxiliary_count,
                                      int(bool(final_flag)), hash_names([final_name]))
                    head = (hash_names([outer]), leading_value,
                            hash_names([inner]), *double_bits,
                            int(marker == b"Object 09"), class_tag,
                            *class_head)
                fourth, cursor = parse_table(data, cursor)
                fourth_total += len(fourth)
                fifth, cursor = parse_table(data, cursor)
                fifth_total += len(fifth)
                short_string, cursor = parse_string(data, cursor)
                prefix_cursor = cursor
                world_leading = struct.unpack_from("<I", data, cursor)[0]
                cursor += 4
                world_double_bits = struct.unpack_from("<Q", data, cursor)[0]
                cursor += 8
                first_block_count = struct.unpack_from("<I", data, cursor)[0]
                cursor += 4
                first_blocks = data[cursor:cursor + 1536 * first_block_count]
                cursor += len(first_blocks)
                second_block_count = struct.unpack_from("<I", data, cursor)[0]
                cursor += 4
                second_blocks = data[cursor:cursor + 256 * second_block_count]
                cursor += len(second_blocks)
                world_class_leading = struct.unpack_from("<I", data, cursor)[0]
                cursor += 4
                world_class, cursor = parse_class_one(data, cursor)
                following_value = struct.unpack_from("<I", data, cursor)[0]
                cursor += 4
                following_double_bits = struct.unpack_from("<Q", data, cursor)[0]
                cursor += 8
                integer_arrays = []
                for width in (4, 12, 4, 1):
                    count = struct.unpack_from("<I", data, cursor)[0]
                    cursor += 4
                    values = data[cursor:cursor + count * width]
                    cursor += len(values)
                    integer_arrays.extend((count, hash_bytes(values)))
                fixed_bytes = data[cursor:cursor + 6144]
                cursor += len(fixed_bytes)
                first_name, cursor = parse_string(data, cursor)
                second_name, cursor = parse_string(data, cursor)
                tail_start = cursor
                for _ in range(32):
                    count = struct.unpack_from("<I", data, cursor)[0]
                    cursor += 4 + count * 48
                tail_hash = hash_bytes(data[tail_start:cursor])
                world_cursor = cursor
                object_marker, cursor = parse_string(data, cursor)
                object_tag = struct.unpack_from("<I", data, cursor)[0]
                cursor += 4
                object_doubles = data[cursor:cursor + 96]
                cursor += len(object_doubles)
                object_name, cursor = parse_string(data, cursor)
                object_final_doubles = data[cursor:cursor + 16]
                cursor += len(object_final_doubles)
                child_count = struct.unpack_from("<I", data, cursor)[0]
                cursor += 4
                child_fields_all = [int(bool(child_count))]
                for _ in range(child_count):
                    parsed_child, cursor = parse_object_child(data, cursor)
                    child_fields_all.extend(parsed_child)
                child_head = tuple(child_fields_all)
                first_flag = data[cursor]
                list_flag = data[cursor + 1]
                cursor += 2
                auxiliary_count = 0
                if list_flag:
                    auxiliary_count = struct.unpack_from("<I", data, cursor)[0]
                    cursor += 4
                child_head += (int(bool(first_flag)), int(bool(list_flag)),
                               auxiliary_count)
                auxiliary_fields, cursor = parse_auxiliary_records(
                    data, cursor, auxiliary_count)
                child_head += auxiliary_fields
                second_flag = data[cursor]
                cursor += 1
                second_marker_matches = 0
                second_tag = 0
                if second_flag:
                    second_marker, cursor = parse_string(data, cursor)
                    second_marker_matches = int(second_marker == b"Object 09")
                    second_tag = struct.unpack_from("<I", data, cursor)[0]
                    cursor += 4
                    second_tags[second_tag] += 1
                    secondary_object_doubles = data[cursor:cursor + 96]
                    cursor += 96
                    secondary_object_name, cursor = parse_string(data, cursor)
                    secondary_object_final_doubles = data[cursor:cursor + 16]
                    cursor += 16
                    secondary_object_children = struct.unpack_from("<I", data, cursor)[0]
                    cursor += 4
                    secondary_object_tail = []
                    for _ in range(secondary_object_children):
                        parsed_child, cursor = parse_object_child(data, cursor)
                        secondary_object_tail.extend(parsed_child)
                    secondary_first_flag, secondary_list_flag = data[cursor:cursor + 2]
                    cursor += 2
                    secondary_auxiliary_count = 0
                    if secondary_list_flag:
                        secondary_auxiliary_count = struct.unpack_from("<I", data, cursor)[0]
                        cursor += 4
                    secondary_auxiliary, cursor = parse_auxiliary_records(
                        data, cursor, secondary_auxiliary_count)
                    secondary_object_tail.extend((int(bool(secondary_first_flag)),
                                                  int(bool(secondary_list_flag)),
                                                  secondary_auxiliary_count,
                                                  *secondary_auxiliary))
                child_head += (int(bool(second_flag)), second_marker_matches,
                               second_tag)
                if second_flag:
                    child_head += (hash_bytes(secondary_object_doubles),
                                   hash_names([secondary_object_name]),
                                   hash_bytes(secondary_object_final_doubles),
                                   secondary_object_children,
                                   *secondary_object_tail)
                object_cursor = cursor
                entity_fields, cursor = parse_entities(data, cursor)
                entity_cursor = cursor
                gadget_fields, cursor = parse_gadgets(
                    data, cursor,
                    lambda data, cursor: parse_object_child(data, cursor, stats=geometry_stats),
                    parse_auxiliary_records,
                    hash_names, entity_stats)
                assert cursor == len(data), (entry.name, cursor, len(data))
                prefix = data[:cursor]
                payload.extend(struct.pack("<I", len(prefix)))
                payload.extend(prefix)
                expected.append((archive.name, entry.name,
                                 (1, first_field, len(first), len(second),
                                  len(third), len(fourth), len(fifth),
                                  object_count, prefix_cursor,
                                  hash_names(first), hash_names(second),
                                  hash_names(third), hash_names(fourth),
                                  hash_names(fifth), hash_names([short_string]),
                                  int(bool(object_count)),
                                  *head, world_leading, world_double_bits,
                                  first_block_count, hash_bytes(first_blocks),
                                  second_block_count, hash_bytes(second_blocks),
                                  world_class_leading, *world_class,
                                  following_value, following_double_bits,
                                  *integer_arrays, hash_bytes(fixed_bytes),
                                  hash_names([first_name]),
                                  hash_names([second_name]), tail_hash,
                                  world_cursor,
                                  int(object_marker == b"Object 09"),
                                  object_tag, hash_bytes(object_doubles),
                                  hash_names([object_name]),
                                  hash_bytes(object_final_doubles),
                                  child_count, *child_head, object_cursor,
                                  *entity_fields, entity_cursor, *gadget_fields, cursor)))
    if not expected:
        parser.error("no compiled levels found in the supplied archives")
    result = subprocess.run([str(args.probe)], input=payload,
                            capture_output=True, check=False)
    if result.returncode != 0:
        offset = 0
        for archive_name, entry_name, _ in expected:
            size = struct.unpack_from("<I", payload, offset)[0]
            frame = payload[offset:offset + size + 4]
            offset += size + 4
            sample = subprocess.run([str(args.probe)], input=frame,
                                    capture_output=True, check=False)
            if sample.returncode != 0:
                parser.error(f"probe failed for {archive_name}:{entry_name} "
                             f"at code {sample.returncode}, size {size}: "
                             f"{sample.stderr.decode(errors='replace')}")
        parser.error(f"probe failed at code {result.returncode}: "
                     f"{result.stderr.decode(errors='replace')}")
    rows = result.stdout.decode().splitlines()
    if len(rows) != len(expected):
        parser.error(f"probe returned {len(rows)} rows for {len(expected)} levels")
    for row, (archive_name, entry_name, target) in zip(rows, expected):
        observed = tuple(int(value) for value in row.split())
        if observed != target:
            parser.error(f"prefix mismatch: {archive_name}:{entry_name}: {observed} != {target}")
    print(f"matched {len(rows)} installed level prefixes, including "
          f"{fourth_total} fourth-table and {fifth_total} fifth-table names; "
          f"world and next-object prefix records also matched; "
          f"second object class tags: {dict(second_tags)}")
    print(f"entity records: {dict(entity_stats)}")
    print(f"entity geometry: {dict(geometry_stats)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
