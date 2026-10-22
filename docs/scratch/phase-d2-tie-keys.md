# D2c: the priority index and the distance reference

Queue id `d2c-tie-keys`. Round 1 of 3. Offline. No stock start. Worker note:
`build/tmp/d2c-tie-keys/round-note.md`. Bytes:
`build/tmp/d2c-tie-keys/reads.txt`.

## Question

What indexes the priority table at `0x004C05D0`, what is the distance
reference at `screen+0x58`, and must two lemmings with the same state vtable
share a priority?

## Answer

The index is the dword at `[state+0x14]`, where `state` is `[lemming+0x340]`.
It is not the vtable. The only instruction that addresses `0x004C05D0` is
`0x004550CC`: `mov edx, dword ptr [eax*4 + 0x4c05d0]`. `eax` is the return of
`0x00430A70`, which is `mov eax, dword ptr [eax+0x14]`. If `[lemming+0x340]`
is 0, the call is skipped and the priority local stays 0 (`0x004550AB`).
There is no bounds check on the index. The displacement `d0 05 4c 00` occurs
once in the file.

The distance reference is three doubles at `screen+0x58`, `+0x60`, and
`+0x68`. The pick adds `0x58` to the screen pointer at `0x004551B1` and
`0x004551CE` and passes it with `lemming+8` to `0x00401830`. That helper
subtracts each reference qword from the matching lemming qword (`+0`, `+8`,
`+0x10`). What point those three doubles name is not in the file. The pick
span does not write them.

Equal state vtables are not forced to share a priority by this load. The
walker constructor `0x0041BD60` writes vtable `0x004B0D28` and the immediate
`0x12` to `[this+0x14]`. File dword index `0x12`, at `0x004C0618`, is 0.
A later store to `[state+0x14]` is not ruled out. Sample 15's winner is
unsettled: both lemmings had that vtable, and their live `+0x14` values and
positions were not saved.

The 48 file dwords at `0x004C05D0` are not a state map. The load has no
length.

## Facilitator check (2026-10-22)

Re-read `0x004550AB` through `0x004550D6`, `0x00430A70` through `0x00430A80`,
`0x004551AB` through `0x004551E6`, `0x00401830` through the three `fsub`
qwords, and `0x0041BD60` through the `[ecx+0x14] = 0x12` store. Each matches
the transcription above. The displacement bytes `d0 05 4c 00` occur once, at
file offset `0x550cf`. File dword index `0x12` is 0. The first twenty table
dwords are `2, 1, 1, 4, 2, 3, 3, 3, 3, 1, 0, 2, 2, 2, 2, 2, 2, 0, 0, 2`.
No Lemmings process was running. No source file changed.

Narrowing: the caller's later stores into `screen+0x58` were not re-derived
here. The subtraction order is lemming minus reference. Sample 15 is still
open. D2 part (a) is not accepted.

**Landed.**
