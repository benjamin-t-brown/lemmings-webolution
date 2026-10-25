# B3e: `MM_CLIMB_002` is not in the executable

Queue `b3e-level-name`. Offline round 1 of 3 on `level-pick`. No stock start. Worker [Find MM_CLIMB_002 name](382bc126-2987-4430-a61d-9c93edc7190c). `prediction.md` 03:08:14, before the search. `round-note.md` 03:10:27. REA's byte search returned no match. Dispatched 03:08 MDT.

## Result

The executable does not contain `MM_CLIMB_002` or `.LVL`. `lemmings.box` has the name at file offset `0x6F77`, inside the path `COMPILED\MM_CLIMB_002.LVL`. That archive string is not a click handler. No code reference was found.

## Facilitator check

`build/tmp/b3e-facilitator/check_name.py` hashes the exe and searches it. The SHA-256 matches `docs/LESSONS.md`. ASCII `MM_CLIMB_002`, its lowercase form, UTF-16LE, and `.LVL` are absent. A read of `lemmings.box` at `0x6F67` is the path `COMPILED\MM_CLIMB_002.LVL`. Expecting the name byte `0x4E` failed; the file byte is `0x4D`. The checker was restored and passed. No Lemmings or Frida process. No source change, so the suite was not rebuilt.

Narrowing: the other archive hits were not opened. Whether code builds this name at runtime was not read. The parked map object was not reopened.

## Stop

Offline round 1 of the level-pick question. The name is an archive path. The selector is the next round. The gate is not scored.
