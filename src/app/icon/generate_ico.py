"""Build the clone's Windows icon from its 32×32 RGBA favicon."""

from pathlib import Path
import struct
import zlib


SIZES = (16, 32, 64, 128, 256)
SOURCE_SIZE = 32


def paeth(left, above, corner):
    estimate = left + above - corner
    return min((left, above, corner), key=lambda value: abs(value - estimate))


def read_pixels(png):
    if png[:8] != b"\x89PNG\r\n\x1a\n" or png[12:29] != b"IHDR" + struct.pack(
        ">IIBBBBB", SOURCE_SIZE, SOURCE_SIZE, 8, 6, 0, 0, 0
    ):
        raise ValueError("expected a 32×32 non-interlaced RGBA8 PNG")
    compressed = bytearray()
    offset = 8
    while offset < len(png):
        length, kind = struct.unpack_from(">I4s", png, offset)
        if kind == b"IDAT":
            compressed.extend(png[offset + 8 : offset + 8 + length])
        offset += 12 + length

    raw = zlib.decompress(compressed)
    stride = SOURCE_SIZE * 4
    if len(raw) != SOURCE_SIZE * (stride + 1):
        raise ValueError("unexpected PNG pixel data length")
    above = bytes(stride)
    pixels = []
    for offset in range(0, len(raw), stride + 1):
        mode = raw[offset]
        if mode > 4:
            raise ValueError("unknown PNG row filter")
        row = bytearray(raw[offset + 1 : offset + 1 + stride])
        for i, value in enumerate(row):
            left = row[i - 4] if i >= 4 else 0
            corner = above[i - 4] if i >= 4 else 0
            predictors = (0, left, above[i], (left + above[i]) // 2)
            predictor = paeth(left, above[i], corner) if mode == 4 else predictors[mode]
            row[i] = (value + predictor) & 255
        pixels.extend(tuple(row[i : i + 4]) for i in range(0, stride, 4))
        above = row
    return pixels


def resize(pixels, size):
    if size >= SOURCE_SIZE:
        scale = size // SOURCE_SIZE
        return [pixels[y // scale * SOURCE_SIZE + x // scale]
                for y in range(size) for x in range(size)]

    # Average 2×2 blocks with premultiplied alpha to avoid color fringes.
    resized = []
    for y in range(0, SOURCE_SIZE, 2):
        for x in range(0, SOURCE_SIZE, 2):
            i = y * SOURCE_SIZE + x
            block = (pixels[i], pixels[i + 1], pixels[i + SOURCE_SIZE], pixels[i + SOURCE_SIZE + 1])
            alpha = sum(pixel[3] for pixel in block)
            rgb = [0, 0, 0]
            if alpha:
                rgb = [(sum(pixel[c] * pixel[3] for pixel in block) + alpha // 2) // alpha
                       for c in range(3)]
            resized.append((*rgb, (alpha + 2) // 4))
    return resized


def bitmap_frame(pixels, size):
    # ICO stores a bottom-up BGRA bitmap followed by a padded 1-bit AND mask.
    header = struct.pack("<IiiHHIIiiII", 40, size, size * 2, 1, 32, 0, 0, 0, 0, 0, 0)
    color = bytearray()
    mask_stride = ((size + 31) // 32) * 4
    mask = bytearray(mask_stride * size)
    for row in range(size):
        start = (size - 1 - row) * size
        for x, (r, g, b, a) in enumerate(pixels[start : start + size]):
            color.extend((b, g, r, a))
            if a == 0:
                mask[row * mask_stride + x // 8] |= 0x80 >> (x % 8)
    return header + color + mask


def make_icon(pixels):
    directory = bytearray(struct.pack("<HHH", 0, 1, len(SIZES)))
    offset = len(directory) + 16 * len(SIZES)
    frames = []
    for size in SIZES:
        frame = bitmap_frame(resize(pixels, size), size)
        directory.extend(struct.pack("<BBBBHHII", size % 256, size % 256, 0, 0,
                                     1, 32, len(frame), offset))
        frames.append(frame)
        offset += len(frame)
    return bytes(directory) + b"".join(frames)


if __name__ == "__main__":
    source = Path(__file__).with_name("webolution.png")
    source.with_suffix(".ico").write_bytes(make_icon(read_pixels(source.read_bytes())))
