"""Independent retained clone-geometry check. Never launches any executable."""
import csv
import json
import math
from pathlib import Path
import sys


def summary(directory):
    return dict(line.split('=', 1) for line in (directory / 'summary.txt').read_text().splitlines())


def rows(directory):
    with (directory / 'selection.csv').open(newline='') as stream:
        return [r for r in csv.DictReader(stream) if r['frame'] == '2']


def main():
    root = Path(sys.argv[1])
    records = []
    selected_directions = set()
    for name in ['upright-left', 'upright-right', 'upright-between', 'upright-overlap',
                 'inverted-left', 'inverted-right', 'inverted-between']:
        before, after = [root / f'pick-nearest-{side}' / name for side in ['before', 'after']]
        b, a = summary(before), summary(after)
        assert b['cursor'] == a['cursor'] and b['nearest'] == a['nearest']
        assert a['saves_unchanged'] == b['saves_unchanged'] == '1'
        assert a['failures'] == b['failures'] == '0'
        old_rows, new_rows = rows(before), rows(after)
        # All measured geometry, state and eligibility are identical before the
        # click. Only the highlight/hover decisions may differ.
        for old, new in zip(old_rows, new_rows, strict=True):
            assert {k: v for k, v in old.items() if k not in ['hovered', 'marked']} == {
                k: v for k, v in new.items() if k not in ['hovered', 'marked']}
        eligible = [r for r in new_rows if r['eligible'] == '1']
        x, y = map(int, a['cursor'].split(','))
        def distance(r):
            return (float(r['centre_x']) - x)**2 + (float(r['centre_y']) - y)**2
        winner = min(eligible, key=lambda r: (distance(r), int(r['actor'])))
        selected_directions.add(int(winner['facing']))
        assert winner['actor'] == a['actual'] == a['nearest']
        assert [r['actor'] for r in new_rows if r['hovered'] == '1'] == [a['actual']]
        assert [r['actor'] for r in new_rows if r['marked'] == '1'] == [a['actual']]
        old_winner = next(r for r in old_rows if r['actor'] == b['actual'])
        old_distance, new_distance = distance(old_winner), distance(winner)
        differs = a['actual'] != b['actual']
        if differs:
            assert new_distance < old_distance
        if name.startswith('upright'):
            assert len(eligible) >= 2 and a['left_facing'] == a['right_facing'] == '1'
            centres = [float(r['centre_x']) for r in eligible]
            if name.endswith('overlap'):
                assert a['overlapping'] == '1'
            else:
                assert (x < min(centres) if name.endswith('left') else
                        x > max(centres) if name.endswith('right') else min(centres) < x < max(centres))
        else:
            assert winner['gravity'] == '-1' and winner['state'] == 'walker'
        old_pixels, new_pixels = [(p / 'hover.png.rgba').read_bytes() for p in [before, after]]
        assert len(old_pixels) == len(new_pixels) == 640 * 480 * 4
        changed = [i // 4 for i in range(0, len(old_pixels), 4)
                   if old_pixels[i:i+4] != new_pixels[i:i+4]]
        # The full-frame count is retained, with selected-sprite changes
        # independently required even if unrelated scenery uses wall-clock seed.
        def inside(pixel, row):
            px, py = pixel % 640, pixel // 640
            return (math.floor(float(row['left'])) <= px < math.ceil(float(row['right'])) and
                    math.floor(float(row['top'])) <= py < math.ceil(float(row['bottom'])))
        actor_changes = sum(inside(p, winner) or inside(p, old_winner) for p in changed)
        if differs:
            assert actor_changes > 0
        records.append(dict(route=name, cursor=[x, y], before=int(b['actual']), after=int(a['actual']),
                            before_distance2=old_distance, after_distance2=new_distance,
                            changed_pixels=len(changed), selected_sprite_changes=actor_changes))
    assert any(r['before'] != r['after'] for r in records)
    assert selected_directions == {-1, 1}, 'actual selections exercise both walking directions'
    (root / 'pick-nearest-comparison.json').write_text(json.dumps(records, indent=2) + '\n')
    print(json.dumps(records, indent=2))


if __name__ == '__main__':
    main()
