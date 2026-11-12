"""Original complete SPR reader with stream/heap boundaries; installed assets read in place."""
import argparse, hashlib, struct, subprocess, sys
from pathlib import Path
from inspect_box import read_index
from verify_reference import EXE_SHA256, pe_file_offset
from run_native_sprite_piece_runtime_oracle import PINS as PIECE_PINS
from run_native_sprite_resource_destruction_oracle import PINS as DESTRUCTION_PINS

PINS = (
    (0x00483EF0, 44, "4085809ff4717a079aaa755c5cc685f3947662685fdc8b39dfbeaea07ffdf74e"),
    (0x00484160, 120, "e86ea86a512f55cfbf60bf5cb30566a23c84f683bcb1871572bf5eb73544f88d"),
    (0x004841E0, 115, "7d0784e4ebe44406f6a8ba6658eaf70e48780ab0b7b4af04d124f0f269ad3ff9"),
    (0x00485800, 56, "88d296305d52000fa827a6c63aa4e9a3e62f094878698fbab3794f7c3da30a22"),
    (0x00485840, 117, "8357651e4a1e9f4c9426e9e18976c2dd4c31ca363eadf879419d7c21792d6df2"),
    (0x00485A80, 27, "b889ac8800cb2d68d5a3ee270635888787a3afdc92b88f27cc172629f14ef9bb"),
    (0x00485AA0, 117, "9f05a8d9ba38c9a50065bb54b7eb52b9122094aaa0633e8500963d4de5e45044"),
    (0x0047C6A8, 210, "7e1446a537f4298771470f35a0482f32b4e0f3750eb8b8ffb4b4235317442e72"),
    (0x0047C643, 101, "6fe386b3451c2143242a73ac9bec30e0c9dacd77ff3e11020b04b595b93e87e3"),
    (0x00484260, 445, "55bb31a43963a9fbdf8ee374f0a89b8dac6b1935be173d4a8c2b4e3bc46e9384"),
    (0x004845F0, 430, "4fa07c68bb6aaa7342351f546486785390cd8973dff095f68d7309d19ed77dc9"),
    (0x004858C0, 433, "30bc2cbcb118b2a996f5f36d23b1bb77f5cb8a6e15e48238e61bf078e66a6bd0"),
    (0x00485B20, 438, "61ac357ed1b11bbc112db65e5a360a91e2a5e43da1db8a450f236113398c87f6"),
    (0x00484420, 78, "05ce7cfb94487d1e8b795191e81debf740b2f227c5858cd6bc4717d9644738cc"),
    (0x00484C10, 22, "90c0aa7faa40cddbc21c573c8301978496fb82ff811a68c2c28dd6a094a55521"),
    (0x0047C5B0, 35, "eb9a915170340852600a26fc3e9885821597850ec73975d7a02a5d4a932d5c0b"),
    (0x00480BB0, 54, "657dbef004ae2a8d230c1d82eb021ca4dfaedfcf59599a6e9c8d666594c0f454"),
    (0x0047C810, 43, "8a0197f8967529dc61c70dbabdbf3d5fda58c65ff481a1b3d974cf37a008fcab"),
    (0x004011A0, 39, "c946d75a8e84b4c660af2c6b118aaef8aba4dbcbe0a5a4e6dd820282e646e700"),
    (0x004844C0, 53, "1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704"),
    (0x00484C30, 53, "1f0030291fa3a68771361e044671fd4c4ed373926b81c41092ccd2071eedc704"),
    (0x0047C7F0, 14, "befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
    (0x00484150, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x004845E0, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x0045B8F0, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x00468CD0, 16, "8857d15e5bce580adb0e5dc96d1aba616e10aa72982c77604b0d7b40d9671936"),
    (0x0048252D, 30, "0e251290ebe7640b7655f043e1ad35014e1cfb34afa25376d6927fd91fe15145"),
    (0x00401B80, 14, "befca35709395b425403c72a706d2f3c715162ad22664d93df30e0255a4d4e8c"),
)

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--exe",type=Path,required=True)
    parser.add_argument("--program",type=Path,required=True)
    parser.add_argument("--archives",type=Path,nargs="*")
    args=parser.parse_args()
    data=args.exe.read_bytes()
    if hashlib.sha256(data).hexdigest()!=EXE_SHA256: parser.error("reference executable mismatch")
    pins={row[0]:(row[0],row[-2],row[-1]) for row in (*PIECE_PINS,*DESTRUCTION_PINS,*PINS)}
    blocks=[]
    for address,size,digest in pins.values():
        offset=pe_file_offset(data,address,size);body=data[offset:offset+size]
        if hashlib.sha256(body).hexdigest()!=digest: parser.error(f"code mismatch {address:08X}")
        blocks.append(struct.pack("<II",address,size)+body)
    payload=struct.pack("<I",len(blocks))+b"".join(blocks)
    sprites=[]
    archives=args.archives if args.archives is not None else [args.exe.parent/"lemmings.box",args.exe.parent/"patch.box"]
    for archive in archives:
        with archive.open("rb") as source:
            for entry in read_index(archive):
                if entry.name.lower().endswith(".spr"):
                    source.seek(entry.offset);body=source.read(entry.size)
                    if len(body)!=entry.size: parser.error("truncated archive entry")
                    sprites.append(struct.pack("<I",len(body))+body)
    payload+=struct.pack("<I",len(sprites))+b"".join(sprites)
    for _ in range(8):
        result=subprocess.run([str(args.program)],input=payload,capture_output=True,timeout=60)
        if result.returncode!=3:
            sys.stdout.buffer.write(result.stdout);sys.stderr.buffer.write(result.stderr)
            return result.returncode
    return result.returncode

if __name__=="__main__":raise SystemExit(main())
