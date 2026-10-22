"""Offline retail panel dispatch checker. No process, REA, or install writes.

Python 3 + Capstone (already installed). The JSON records file-backed bytes,
decoded source operands, two distinct dispatch tables, and command consumers.
REA structural evidence is separately retained in the C1a scratch packet.
--self-test mutates only an in-memory factory-table dword, bypassing the digest
gate so that a semantic/table assertion, not SHA-256 alone, must reject it.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
from pathlib import Path

from capstone import Cs, CS_ARCH_X86, CS_MODE_32
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_EBP

AUTHORITY = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
DEFAULT_EXE = Path(r"C:\progs\Lemmings Revolution\Lemmings Revolution.exe")
CS = Cs(CS_ARCH_X86, CS_MODE_32)
CS.detail = True


def hx(n):
    return f"0x{n:08x}"


def require(condition, message):
    if not condition:
        raise ValueError(message)


class PE:
    def __init__(self, data):
        self.data = bytes(data)
        pe = struct.unpack_from("<I", self.data, 0x3C)[0]
        require(self.data[pe:pe + 4] == b"PE\0\0", "PE signature")
        machine, count = struct.unpack_from("<HH", self.data, pe + 4)
        require(machine == 0x14C, "not x86 PE")
        opt_size = struct.unpack_from("<H", self.data, pe + 20)[0]
        opt = pe + 24
        require(struct.unpack_from("<H", self.data, opt)[0] == 0x10B, "not PE32")
        self.base = struct.unpack_from("<I", self.data, opt + 28)[0]
        require(self.base == 0x400000, "unexpected image base")
        self.sections = []
        for j in range(count):
            pos = opt + opt_size + 40 * j
            name = self.data[pos:pos + 8].rstrip(b"\0").decode("ascii")
            virtual_size, rva, raw_size, offset = struct.unpack_from("<IIII", self.data, pos + 8)
            self.sections.append(dict(name=name, va=self.base + rva,
                                      virtual_size=virtual_size, raw_size=raw_size,
                                      file_offset=offset))

    def offset(self, va, size):
        for s in self.sections:
            if s["va"] <= va and va + size <= s["va"] + s["raw_size"]:
                result = s["file_offset"] + va - s["va"]
                require(result + size <= len(self.data), "truncated file")
                return result
        raise ValueError(f"not file-backed: {hx(va)} length {size}")

    def read(self, va, size):
        off = self.offset(va, size)
        return self.data[off:off + size]

    def u32(self, va):
        return struct.unpack("<I", self.read(va, 4))[0]

    def string(self, va):
        out = bytearray()
        for j in range(96):
            ch = self.read(va + j, 1)[0]
            if not ch:
                return out.decode("ascii")
            out.append(ch)
        raise ValueError("unterminated string")

    def code(self, va, size):
        return list(CS.disasm(self.read(va, size), va))

    def record(self, va, size, instructions=False):
        result = dict(address=hx(va), file_offset=hx(self.offset(va, size)),
                      bytes=self.read(va, size).hex(" "))
        if instructions:
            result["instructions"] = [ins_record(i) for i in self.code(va, size)]
        return result


def ins_record(i):
    return dict(address=hx(i.address), bytes=i.bytes.hex(" "),
                mnemonic=i.mnemonic, operands=i.op_str)


def direct_calls(code):
    return [i for i in code if i.mnemonic == "call" and i.operands[0].type == X86_OP_IMM]


def imm(i):
    return i.operands[-1].imm & 0xFFFFFFFF


def pin(image, address, expected):
    expected = bytes.fromhex(expected)
    require(image.read(address, len(expected)) == expected, f"byte pin {hx(address)}")
    return image.record(address, len(expected), True)


# Exact command/connection operands, not a claim that these execute in a live run.
PINS = {
    0x45A119: "8b 48 28", 0x45A11F: "83 7d f8 09",
    0x45A12C: "ff 24 95 dd a4 45 00", 0x45A35B: "32 c0",
    0x45B015: "8b 48 28", 0x45B01E: "83 3c 8a 00",
    0x45B035: "8b 48 28", 0x45B03E: "83 bd 70 ff ff ff 08",
    0x45B045: "0f 87 10 05 00 00", 0x45B051: "ff 24 95 70 b5 45 00",
    0x45B028: "c7 45 f0 00 00 00 00", 0x45B55B: "8b 45 f0",
    0x45A994: "8b 14 8d 94 06 4c 00",
    0x455670: "b9 d8 91 4c 00", 0x455675: "e8 71 59 00 00",
    0x455696: "e8 45 4a 00 00",
    0x4556E6: "e8 ab 8f fd ff", 0x4556F0: "e8 9f 5e 00 00",
    0x45ACE4: "89 48 28", 0x455B12: "83 79 38 08",
    0x455B16: "75 21", 0x455B30: "c7 40 20 01 00 00 00",
    0x455B58: "e8 ff 50 00 00", 0x45DB1A: "83 78 20 00",
    0x45DB20: "8b 0d f8 15 4b 00", 0x45DB2B: "c7 45 c8 01 00 00 00",
    0x45DB41: "8b 4d c8", 0x45DB47: "83 ea 01",
    0x45472B: "83 79 38 09", 0x454739: "e8 d2 16 00 00",
    0x454747: "a1 00 92 4c 00", 0x45475B: "e8 34 6e 00 00",
    0x454765: "e8 2e 03 ff ff", 0x444AB1: "c6 82 38 51 00 00 01",
    0x442646: "8a 82 38 51 00 00", 0x413CC6: "c6 40 48 01",
    0x417E96: "c6 40 49 01", 0x45B1B1: "6a 00 6a 05",
    0x413D74: "db 45 08", 0x413D7A: "dd 58 08",
    0x45A5AD: "c7 40 2c 08 00 00 00", 0x45B8AC: "89 41 2c",
}
ELIGIBILITY = [0x45A202, 0x45A2D1, 0x45A28C, 0x45A178, 0x45A1BD,
               0x45A133, 0x45A316, 0x45A247, 0x45A362, 0x45A35B]
FACTORY = [0x45B288, 0x45B1EA, 0x45B197, 0x45B239, 0x45B0F6,
           0x45B058, 0x45B0A7, 0x45B148, 0x45B2D7]
CTORS = [0x413C70, 0x417E40, 0x413D40, 0x40DFB0, 0x40F8D1,
         0x40BBA0, 0x418470, 0x415850]
LABELS = ["climb", "float", "bomb", "block", "build", "bash", "mine", "dig",
          "questionmark", "armageddon"]


def table(image, dispatch, count):
    i = image.code(dispatch, 7)[0]
    require(i.mnemonic == "jmp" and i.operands[0].type == X86_OP_MEM,
            "dispatch is not an indirect jump")
    m = i.operands[0].mem
    require(m.scale == 4, "not dword indexed")
    targets = [image.u32(m.disp + 4 * j) for j in range(count)]
    return m.disp, targets, dict(dispatch=ins_record(i), count=count,
                               **image.record(m.disp, count * 4),
                               targets=[hx(t) for t in targets])


def constructor(image, va):
    code = image.code(va, 0x90)
    vt = next(i for i in code if i.mnemonic == "mov" and
              len(i.operands) == 2 and i.operands[0].type == X86_OP_MEM and
              i.operands[0].mem.disp == 0 and i.operands[1].type == X86_OP_IMM and
              0x4B0000 <= imm(i) < 0x4BF000)
    tag = next(i for i in code if i.mnemonic == "mov" and
               i.operands[0].type == X86_OP_MEM and
               i.operands[0].mem.base != X86_REG_EBP and
               i.operands[0].mem.disp == 0x14 and i.operands[1].type == X86_OP_IMM)
    vtable = imm(vt)
    return dict(address=hx(va), vtable=hx(vtable), type=imm(tag),
                vtable_store=ins_record(vt), type_store=ins_record(tag),
                vtable_bytes=image.record(vtable, 24),
                step=hx(image.u32(vtable + 4)), enter=hx(image.u32(vtable + 8)))


def extract(image):
    # Decode displacements from actual instructions, not assumed table addresses.
    eaddr, elig, er = table(image, 0x45A12C, 10)
    faddr, factory, fr = table(image, 0x45B051, 9)
    name_ref = image.code(0x45A994, 7)[0]
    names_address = name_ref.operands[1].mem.disp
    rows = []
    for j in range(10):
        name_ptr = image.u32(names_address + 4 * j)
        row = dict(index=j, shipped_art_key=image.string(name_ptr),
                   name_pointer=image.record(names_address + j * 4, 4),
                   name_bytes=image.record(name_ptr, len(image.string(name_ptr)) + 1),
                   eligibility_target=hx(elig[j]))
        ecalls = direct_calls(image.code(elig[j], 0x20))
        if j != 9:
            row["eligibility_first_call"] = ins_record(ecalls[0])
        else:
            row["eligibility_false"] = image.record(elig[j], 7, True)
        if j < 8:
            calls = direct_calls(image.code(factory[j], 0x40))
            require(imm(calls[0]) == 0x49C1A8, "factory allocation call")
            ctor = imm(calls[1])
            row.update(factory_target=hx(factory[j]), constructor_call=ins_record(calls[1]),
                       constructor=constructor(image, ctor),
                       action=row["shipped_art_key"], evidence_kind="READ, not live measured")
        elif j == 8:
            row.update(action="fast-forward command (seven simulation passes)",
                       physical_handler=hx(0x455B12), flag_store=hx(0x455B30),
                       consumer=hx(0x45DB1A), passes=image.u32(0x4B15F8),
                       latent_factory_target=hx(factory[j]),
                       scope="Physical panel bypasses selection/factory for 8; separate latent +0x2c skill path below")
        else:
            row.update(action="Armageddon / mass explosion command", physical_handler=hx(0x45472B),
                       request_call=hx(0x454765), request_target=hx(0x444A98),
                       flag="game+0x5138", consumer=hx(0x442646),
                       scope="No per-lemming factory case 9; native handler tests stock[selected], not necessarily stock[hovered]")
        rows.append(row)
    return rows, elig, factory, er, fr, eaddr, faddr


def check(image, digest_gate=True):
    if digest_gate:
        require(hashlib.sha256(image.data).hexdigest() == AUTHORITY, "authority SHA-256")
    rows, elig, factory, er, fr, ea, fa = extract(image)
    require(ea == 0x45A4DD and fa == 0x45B570, "source table displacements")
    require(elig == ELIGIBILITY, "eligibility table targets changed")
    require(factory == FACTORY, "factory table targets changed")
    require([r["shipped_art_key"] for r in rows] == LABELS, "shipped label order")
    require([int(r["constructor"]["address"], 16) for r in rows[:8]] == CTORS,
            "factory index -> constructor mismatch")
    require([r["constructor"]["type"] for r in rows[:8]] == [0, 1, 0x1F, 3, 4, 5, 6, 7],
            "constructor state type mismatch")
    data_section = next(s for s in image.sections if s["name"] == ".data")
    require(data_section["va"] + data_section["raw_size"] == 0x4C6000, ".data raw end")
    pins = [pin(image, a, b) for a, b in PINS.items()]
    require(rows[8]["passes"] == 7, "fast-forward pass count")
    mass_calls = direct_calls(image.code(0x442644, 0x3EE))
    mass_ctor_call = next(i for i in mass_calls if imm(i) == 0x416910)
    mass_attach_call = next(i for i in mass_calls if imm(i) == 0x42E696)
    rows[9]["mass_state_constructor_call"] = ins_record(mass_ctor_call)
    rows[9]["mass_state_constructor"] = constructor(image, imm(mass_ctor_call))
    rows[9]["mass_state_attach_call"] = ins_record(mass_attach_call)
    # The key runtime globals are deliberately not read out of zero-fill/.rsrc.
    for a in [0x4C9200, 0x4CBB40]:
        try:
            image.read(a, 1)
        except ValueError:
            pass
        else:
            raise ValueError("runtime-only global erroneously file-backed")
    special = []
    for cmpva, callva, ctor in [(0x45B2DD, 0x45B300, 0x41A520),
                               (0x45B338, 0x45B35B, 0x41CBC0),
                               (0x45B393, 0x45B3B6, 0x41B630),
                               (0x45B3EE, 0x45B411, 0x40BAC0),
                               (0x45B449, 0x45B46C, 0x41B630),
                               (0x45B4A4, 0x45B4C7, 0x41AC50),
                               (0x45B4FC, 0x45B528, 0x416730)]:
        ci = image.code(cmpva, 4)[0]
        require(ci.mnemonic == "cmp" and ci.operands[0].mem.disp == 0x2C, "special selector")
        call = image.code(callva, 5)[0]
        require(call.mnemonic == "call" and imm(call) == ctor, "special constructor call")
        special.append(dict(selector=imm(ci), compare=ins_record(ci), call=ins_record(call),
                            constructor=constructor(image, ctor)))
    # Small relevant byte ranges, decoded from genuine instruction boundaries.
    regions = [(0x455AFF, 0x62), (0x45472B, 0x78), (0x455657, 0xA0),
               (0x45DB0E, 0x253), (0x444A98, 0x26), (0x442644, 0x3EE),
               (0x45ACDE, 0x9), (0x413CBC, 0x12), (0x417E8C, 0x2A),
               (0x45B1B1, 0xC), (0x45A35B, 7), (0x45B2D7, 0x284)]
    return dict(authority_sha256=AUTHORITY, file_sha256=hashlib.sha256(image.data).hexdigest(),
                image_base=hx(image.base), sections=image.sections,
                tables=dict(eligibility=er, factory=fr), mapping=rows,
                latent_index8_special_factory=special,
                latent_limits="+0x2c default 8 and serialized load; duplicate selector 14 later branch is unreachable in this else-if chain; no retail reachability or live assignment claimed",
                byte_pins=pins, source_regions=[image.record(a, n, True) for a, n in regions])


def self_test(path, baseline):
    original = path.read_bytes()
    image = PE(original)
    before_script = hashlib.sha256(Path(__file__).read_bytes()).hexdigest()
    mutated = bytearray(original)
    # Index 5 alone now points at the perfectly valid index 6 factory branch.
    offset = image.offset(0x45B570 + 5 * 4, 4)
    struct.pack_into("<I", mutated, offset, image.u32(0x45B570 + 6 * 4))
    altered = PE(mutated)
    wrong = extract(altered)[0][5]
    require(wrong["constructor"]["address"] == hx(CTORS[6]), "mutation did not affect extraction")
    try:
        check(altered, digest_gate=False)
    except ValueError as e:
        rejection = str(e)
        require(rejection == "factory table targets changed", "unexpected rejection")
    else:
        raise ValueError("synthetic index mutation escaped")
    require(path.read_bytes() == original, "authority file changed")
    require(hashlib.sha256(Path(__file__).read_bytes()).hexdigest() == before_script,
            "checker script changed")
    require(check(PE(path.read_bytes())) == baseline, "baseline not restored")
    return dict(passed=True, method="one in-memory factory[5] dword replaced by factory[6]",
                digest_gate_bypassed=True, table_address=hx(0x45B584),
                before=image.record(0x45B584, 4), after=altered.record(0x45B584, 4),
                extracted_wrong_constructor=wrong["constructor"]["address"], rejected_by=rejection,
                authority_file_unchanged=True, script_unchanged=True,
                baseline_recheck_identical=True, script_sha256=before_script,
                restoration="Discarded synthetic bytearray; no on-disk binary or script mutation")


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--exe", type=Path, default=DEFAULT_EXE)
    ap.add_argument("--output", type=Path)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    result = check(PE(args.exe.read_bytes()))
    result["exe"] = str(args.exe)
    if args.self_test:
        # Compare baseline without the display-only exe field.
        baseline = dict(result)
        del baseline["exe"]
        result["negative_control"] = self_test(args.exe, baseline)
    text = json.dumps(result, indent=2) + "\n"
    if args.output:
        args.output.write_text(text, encoding="utf-8", newline="\n")
        print(json.dumps(dict(output=str(args.output), mapping=[dict(index=r["index"], action=r["action"])
                                                               for r in result["mapping"]],
                              negative_control=result.get("negative_control"))))
    else:
        print(text, end="")


if __name__ == "__main__":
    main()
