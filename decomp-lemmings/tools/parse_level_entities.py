"""Independent layout check for the entity tail of installed Level 23 files.

This is a fixture oracle derived from the executable's read calls. It does
not construct game objects or model the loader's non-stream side effects.
"""
import struct


def parse_gadgets(data, cursor, parse_child, parse_auxiliary, hash_names, stats):
    class Reader:
        def __init__(self):
            self.cursor = cursor
            self.fields = []

        def scalar(self, fmt="I", flag=False):
            value = struct.unpack_from("<" + fmt, data, self.cursor)[0]
            self.cursor += struct.calcsize("<" + fmt)
            if flag:
                value = int(bool(value))
            self.fields.append(value)
            return value

        def numbers(self, count, fmt="I"):
            return [self.scalar(fmt) for _ in range(count)]

        def flag(self):
            return self.scalar("B", True)

        def text(self, capacity=128, marker=False):
            length = struct.unpack_from("<I", data, self.cursor)[0]
            self.cursor += 4
            assert length < capacity and length <= len(data) - self.cursor
            value = data[self.cursor:self.cursor + length].split(b"\0", 1)[0]
            self.cursor += length
            self.fields.append(int(value == b"Object 09") if marker else hash_names([value]))
            return value

        def sequence(self, read, key=None):
            count = self.scalar()
            assert count <= len(data) - self.cursor
            if key:
                stats[key] += count
            for _ in range(count):
                read()

        def model(self):
            tag = self.scalar()
            stats[f"model_{tag}"] += 1
            if tag == 0:
                self.text(255)
                self.numbers(3, "Q")
                self.text(255, marker=True)
                fields, self.cursor = parse_child(data, self.cursor)
                self.fields.extend(fields)
                self.flag()
                if self.flag():
                    count = self.scalar()
                    fields, self.cursor = parse_auxiliary(data, self.cursor, count)
                    self.fields.extend(fields)
                if self.flag():
                    self.text()
            elif tag == 1:
                self.numbers(4)
                self.text(255)
            elif tag == 2:
                self.numbers(7, "Q")
                self.scalar()
                self.sequence(lambda: self.numbers(10, "Q"), "model_array")
                self.numbers(2)
                self.text()
                self.text()
                self.sequence(self.model, "nested_models")
            else:
                raise ValueError(f"unknown model tag {tag} at {self.cursor - 4}")

        def base(self):
            self.numbers(5, "Q")
            self.flag()
            self.flag()
            self.scalar("H")

        def basic(self):
            self.base()
            self.flag()
            present = self.flag()
            self.flag()
            if present:
                self.model()

        def spatial(self):
            self.basic()
            self.numbers(6, "Q")
            for _ in range(2):
                self.sequence(lambda: self.numbers(3, "Q"))

        def linked(self):
            self.basic()
            self.numbers(2, "Q")
            self.scalar()
            self.text()
            self.text()
            self.numbers(12, "Q")
            self.scalar()

        def read_425e50(self):
            self.spatial()
            self.scalar()
            self.numbers(3, "H")
            self.numbers(3)
            self.flag()
            self.scalar("H")
            self.flag()
            self.flag()

        def read_426c6b(self):
            self.spatial()
            self.scalar()
            self.scalar(flag=True)

        def read_424187(self):
            self.base()
            self.flag()
            self.numbers(10)

        def read_42acac(self):
            self.spatial()
            self.model()
            for _ in range(2):
                if self.flag():
                    self.model()
            self.numbers(3)
            self.numbers(3, "Q")
            self.scalar()
            self.numbers(3, "Q")
            self.text()
            self.numbers(30, "Q")
            self.text(260)
            self.scalar()

        def read_4347ec(self):
            self.spatial()
            self.model()
            self.model()
            self.scalar()
            self.numbers(4, "Q")
            for _ in range(4):
                self.text()

        def tagged(self):
            tag = self.scalar()
            stats[f"tagged_{tag}"] += 1
            assert tag in range(5)
            self.spatial()
            self.scalar("Q")
            self.flag()
            self.flag()
            self.scalar()
            if tag <= 2:
                for _ in range(1 if tag == 0 else 2):
                    self.sequence(lambda: self.numbers(4), "tagged_records")
            else:
                self.scalar()
                if tag == 4:
                    self.numbers(4)

        def named(self):
            tag = self.text(255)
            stats[f"named_{tag.decode()}"] += 1
            assert tag in (b"STAT", b"DYNA", b"FALL")
            if tag != b"STAT":
                self.text(255)
            self.spatial()
            self.flag()
            self.scalar("Q")
            self.scalar()
            self.flag()
            self.flag()
            for _ in range(4):
                self.text()
            self.flag()
            self.flag()
            if tag == b"DYNA":
                self.numbers(4, "Q")
                self.numbers(4)
                self.numbers(2, "Q")
                self.flag()
            elif tag == b"FALL":
                self.text()
                self.text()
                self.numbers(2, "Q")
                self.text()
                self.text()

    reader = Reader()
    for read, key in ((reader.basic, "basic"),
                      (reader.linked, "linked"),
                      (reader.read_425e50, "425e50")):
        reader.sequence(read, key)
    reader.scalar()
    for read, key in ((reader.read_426c6b, "426c6b"),
                      (reader.spatial, "spatial"),
                      (reader.read_424187, "424187"),
                      (reader.read_42acac, "42acac"),
                      (reader.read_4347ec, "4347ec"),
                      (reader.basic, "4342bf"),
                      (reader.spatial, "41e038"),
                      (reader.tagged, "tagged"),
                      (reader.named, "named")):
        reader.sequence(read, key)
    reader.numbers(11)
    reader.sequence(reader.scalar, "tail_integers")
    for _ in range(5):
        reader.flag()
    reader.scalar("H")
    reader.scalar()
    reader.sequence(lambda: reader.numbers(3), "tail_records")
    return tuple(reader.fields), reader.cursor
