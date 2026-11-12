# Inline actor model construction

Authority: installed executable SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
The original constructor chain is recovered concretely for normal execution.
No allocation, resource acquisition, rendering, or other external callback is
invented by this initializer.

| Routine | VA / file offset | Bytes | SHA-256 |
| --- | --- | ---: | --- |
| Actor inline model | `0x43CE00` / `0x3CE00` | 136 | `234d6c9a8f7339d98fa0eff288e46d5defcabb5544914f52c54084dbe5adfaab` |
| Model base | `0x465840` / `0x65840` | 119 | `45be7b301be996428b11a2a7d531d49a8294198ba2a1661ffbc906503725b667` |
| Animation model | `0x4680F0` / `0x680F0` | 285 | `e52c82772f8e310caa379fc7a8fe694a75024e1e218482e40eaa25ac6e4549d9` |
| First-byte clear | `0x41A490` / `0x1A490` | 20 | `8604081cbfaca3bd2be59e14d7329270bd8c943ec93d07607b4d57e4713e05c1` |
| Animation base | `0x4409E0` / `0x409E0` | 35 | `bc997c93547cee16c683ff9498104d940c6ac2e8a5cb9de58ecc9465a20ace44` |
| Quad initialization | `0x440A10` / `0x40A10` | 469 | `cec018a6b0b0958089c1a62379859f94c33673631d3524a87b73bd4fbb8a32ac` |
| Empty corner constructor | `0x420D60` / `0x20D60` | 55 | `ebc365e89b779b7dcb472eec894880fd1194961acea6a94f9d6734d02d8f37f5` |
| Empty corner destructor | `0x420DA0` / `0x20DA0` | 11 | `012a7b6ce927f083ba98a546e3d3f750b2a1d82f350ca27efbd30a9c3c7718a5` |
| CRT vector constructor | `0x49C5D3` / `0x9C5D3` | 106 | `b80af3235f4307f2af0c9cb3c24267eb2cba55ad9a856b8d070ddbb17f7483d5` |
| CRT success cleanup funclet | `0x49C63D` / `0x9C63D` | 24 | `3f6add3a0b9e7cae0d44ae4404274273921e43355dbfca561cc807194b6fba45` |

Original instruction bytes: [model_construction.asm](model_construction.asm).
Previously pinned helpers `4017F0`, `401B80`, and `4821D0` are also used unchanged.

## Static evidence

REA procedure information/pseudocode, corroborated by local disassembly:

- `43CE00`: `ev_c3d026d8ca16a4771af36be1cc4d7a89ea0d28f6b9e1f4634a2b0ffd7816e1c6`
- `465840`: `ev_b4c01bc0fb6d247e1e594e4819b1c781f1d3c34aea73ad53b7d916097c255dde`
- `4680F0`: `ev_31aa1ea745911b22ff6ea43551f44bbaf13c4eae9c147a2935fc34c19da2820c`
- `41A490`: `ev_4275f6e31aa16e7cf111fc9103b8f4721cc7bfaa56c8d39ed135d57a26463b2a`
- `4409E0`: `ev_f59e797c4c889cab82d011ddc5cac7769933e08005ba2d5e5bdb23dd587f8b0a`
- `440A10`: `ev_a6ee655b0baa4b45770e5118c067696437b28974247b04b262508c50b625ec3d`
- `420D60`: `ev_184a9d65304eed90d5efa137d17fbf31fd2455546ce7cc46e1a977ede4f9a153`
- `420DA0`: `ev_84bcaac6e18695e988c68377ef2addc387b2cc290159cbed7568ac0b686c6b3e`
- `49C5D3`: `ev_55ba760be88827506de307fa56d886cccbaff9b66dd3de4f6bb4bb57f6b97f64`
- `49C63D`: `ev_8079adae3643625d89f500cae293e02d47df678755f844fc1dc26a8444d6c8ec`

## Recovered fields and composition

Model offsets below are relative to its inline object, which is actor `+78`.
`initialize_model_base` (`465840`) clears only the first byte of the component
at `+8` through `41A490`; its remaining 127 bytes are preserved. The empty
position constructor leaves triple `+90` untouched. It writes vtable `4B1858`,
field `+A8=1`, and mode `+88=3`.

`initialize_actor_model` (`43CE00`) constructs that base, then constructs the
animation object at model `+B0`. Afterwards it writes vtable `4B14A8`, byte
`+2B8=1`, DWORDs `+2C0=0,+2C4=0`, and replaces mode `+88` with 1.

Animation-relative offsets:

- `440A10` constructs four `0x68`-byte corner records at `+8`. Their nested
  triple/pair constructors (`420D60` calling empty `401B80`/`4821D0`) preserve
  storage. It then writes DWORD `+0=0`, the bit pattern `3F6D7DBF487FCB92`
  (binary64 0.0036) at each corner `+10`, triple `(255,255,255)` at each corner
  `+38`, and double 255 at each corner `+50`. No other corner fields are reset.
- `4409E0` invokes `440A10` and writes field `+1A8=-1`.
- `4680F0` invokes that base, leaves the empty `+1E0` triple constructor without
  a write, then sets direction `+1D0=1`, finished `+200=0`, double `+1D8=+0`,
  resource pointer `+1F8=null`, clip `+1C8=0`, frame `+1CC=0`, fraction
  `+1C0=+0`, DWORD `+1B0=0`, looping `+201=1`, resource ID `+1FC=-1`, DWORD
  `+1B8=0`, and triple `+1E0=(+0,+0,-1)` in that order.

The animation fields already used by playback reuse `AnimationPlayback`.
Unknown fields retain offset names. Unknown corner doubles are stored as bit
patterns so uninitialized representations and signaling NaNs are preserved.
The portable views are not asserted to match original ABI layout. Their default
C++ values are not claims about original initialization of untouched storage.

The original CRT array helper `49C5D3` calls the empty corner constructor four
times with stride `0x68`. On normal completion it marks success and invokes
`49C63D`, whose success branch returns without destruction. Source composition
omits this empty work. Exceptional cleanup through the alternate funclet branch
is not recovered; no general CRT vector-construction implementation is claimed.

## Tests and native evidence

Units pass on x64 and x86 at O0/O2. They check base and actor model mode/vtable
differences, all playback defaults, exact constant bits, resource pointer reset,
positive zero, first-byte-only clearing, unchanged model position, preserved
corner components/gaps, and standalone animation-base behavior.

The native runner verifies executable identity, all 13 loaded routine spans,
and the original constant bytes `92 cb 7f 48 bf 7d 6d 3f` at `4B15C0`.
It executes **unchanged original code without adapters or patched call sites**,
including the CRT array helper, all four corner constructors, and the success
cleanup funclet. Original SEH frames are installed/restored normally.

**20,000 complete-buffer comparisons pass at both O0 and O2**, with 4,000 calls
to each of `440A10`, `4409E0`, `4680F0`, `465840`, and `43CE00`. Every trial
starts with random bytes and compares all `0x2C8` model bytes plus constructor
return identity. Explicit signaling NaNs exercise untouched position/pair bits;
random padding and every other untouched field must remain identical.

This establishes constructor initialization on the normal path. No original
game rendering, resource loading, exception unwind, or arbitrary preexisting
object destruction is exercised. The initializer does not release a preexisting
resource because the original constructor does not do so.

```powershell
python tools/run_native_model_construction_oracle.py --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' --program build/x86/native_model_construction_oracle.exe
```
