"""Check the diagnostic tool's runtime texture-name precondition with synthetic data."""
import argparse
import json
from pathlib import Path
import struct
import subprocess
import tempfile


def single_sprite_box(texture_length):
    texture = b'T' * texture_length
    sprite = (struct.pack('<III', 1, 1, 1) + bytes(64)
              + struct.pack('<I', len(texture)) + texture
              + struct.pack('<I', 4) + b'walk' + struct.pack('<dI', 1.0, 0))
    name = b'SPRITES\\REGRESSION.SPR'
    names = struct.pack('<I', len(name)) + name
    offset = 14 + len(names) + 16
    return (b'LEMBOX' + struct.pack('<II', 1, len(names)) + names
            + struct.pack('<IIII', 1, offset, 1, len(sprite)) + sprite)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--program', type=Path, required=True)
    args = parser.parse_args()
    program = args.program.resolve()
    # For normal build-tree invocation, all generated data stay in ignored build/.
    with tempfile.TemporaryDirectory(prefix='resource_boundary_', dir=program.parent) as temporary:
        for length in (127, 128):
            archive = Path(temporary) / f'texture_{length}.box'
            archive.write_bytes(single_sprite_box(length))
            result = subprocess.run([str(program), '--archive', str(archive), '--all'],
                                    capture_output=True, text=True, timeout=15)
            if length == 127:
                if result.returncode != 0:
                    raise AssertionError(f'valid 127-byte name failed: {result.stderr}')
                observed = json.loads(result.stdout)
                expected = {
                    'resources': 1, 'clips': 1, 'frames': 1, 'pieces': 1,
                    'created': 3, 'destroyed': 3, 'animation_checks': 1,
                    'loaded_after_cleanup': 0, 'texture_names': 1,
                    'stream_reads': 3, 'stream_closes': 3, 'stream_seeks': 3,
                    'quads_checked': 1,
                    'slot_reused': True,
                }
                if observed != expected:
                    raise AssertionError(f'{observed} != {expected}')
            elif result.returncode != 1 or 'texture name exceeds original registration slot buffer' not in result.stderr:
                raise AssertionError(f'invalid 128-byte name was not rejected cleanly: {result}')
    print('Synthetic 127-byte texture lifecycle and 128-byte name rejection passed')


if __name__ == '__main__':
    main()
