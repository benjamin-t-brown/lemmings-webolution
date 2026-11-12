# Emitter update at 0x00425A95

## Identity and bytes

Authority: installed `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.
This executable includes the installed patch integration. The sibling remake
was not used as evidence.

| Address | File offset | Length | SHA-256 |
| --- | ---: | ---: | --- |
| `0x00425A95` | 154261 | 728 | `f0babcbc6518c418763f529e6ff209728216e2c855fd3d57297951f9ebf47bce` |
| `0x004B0338` | 721720 | 8 | `af5570f5a1810b7af78caf4bc70a660f0df51e42baf91d4de5b2328de0e83dfc` |
| `0x004B0F48` | 724808 | 8 | `3a782dc88acafd2e210287a7ccbcefda1d89207e71b059f4c28ba42c72e220e8` |

The constants are binary64 zero (`0000000000000000`) and 1.3
(`CDCCCCCCCCCCF43F`, bytes in file order). The runner checks the full executable
digest and PE section mapping before passing these exact byte ranges directly
to the native test process. No executable or asset is stored in the repository.

REA pseudocode evidence:
`ev_cb5060d61687d55b5dcc52a522a3fb4632e98808e7dda30886a946fb2b80bc41`.
Procedure extent is `0x00425A95..0x00425D6C` inclusive. Local PE disassembly
independently checked the calling conventions, branches, widths, and stores.

## Recovered behavior

This is vtable `0x004B0F50` slot `+0x14`, for the entity loaded by `0x00425E50`.
The portable `EmitterState` is a logical view, with original offsets in field
names. It is not an assertion about the host C++ object layout.

1. When local byte `+AE` and global byte `0x004E32A0` are both zero, call
   `0x0044521B(global_4E32B4, 0)`, then set `+AE=1`. This happens even when no
   actors remain. Suppressing the call leaves `+AE` unchanged.
2. While byte `+AC` is zero, call `0x00432383(this, elapsed)`, then model slot
   `+44`. On a nonzero result, set `+AC=1`, replace elapsed with model slot
   `+48`'s binary64 result, and set the countdown `+C0` to binary64 1.3.
3. If `+AC` is nonzero and signed short `+98` is positive, subtract elapsed
   from `+C0`, storing binary64 before comparison. The original uses
   `FCOMP; FNSTSW AX; TEST AH,0x41` at `0x00425B9F..0x00425BAA`. **Less,
   equal, and unordered all emit.** In particular, NaN emits; replacing this
   with an ordinary `countdown <= 0` would change behavior.
4. Decrement `+98` before allocating `0x3A0` bytes. Constructor `0x0042DE51`
   receives signed shorts `+9A` and `+9C` converted to doubles and the signed
   32-bit value `+B0`. Nonzero byte `+AD` invokes `0x0042F5E3`; direction
   short `+B4==-1` invokes `0x0042F66B`. Byte `+B6` must equal **exactly 1**
   to change `+B4` from 1 to -1, or from any other value to 1.
5. Call new actor slot `+14` with the negated countdown (the elapsed excess).
   A zero result calls `0x004090B0` then `0x0043B536`. A nonzero result calls
   `0x00426180` with a pointer to the local actor handle. The latter interface
   can replace that handle; subsequent reads use the resulting handle.
6. Decrement the 16-bit counter indexed by the actor's current signed field
   `+4C` at `0x004CBA8C`, then the 16-bit total at `0x004CBAA0`, with wrap.
   Add interval `+B8` to countdown `+C0` and call `0x00444ABC`.
7. Return low byte 1. There is no catch-up loop: one update emits at most one
   actor even when the final countdown remains negative.

Supplementary static evidence names the optional actor operations:

- `0x0042F5E3` negates actor `+44`, sets byte `+5C`, and changes the vertical
  components at `+358/+368`. REA:
  `ev_f39026e35582ad9603d7c88a523eadd0550fb744d639b5820c3dc25be35fc4d1`.
- `0x0042F66B` negates actor `+40` and changes components `+350/+360`. REA:
  `ev_b923a563be17e741a308e815842be2aee032a8cff5146d8aa37c5dc12e346c30`.

Those functions remain callback boundaries here; their state changes are not
implemented by this slice. The model, allocation, construction, sound/effect,
containers, and UI refresh are also explicit callbacks. The actor-type callback
is a host projection of the original direct `+4C` read, not an original call.

## Verification

**Static reading:** REA and original instruction inspection establish the
routine and callback order above. Unknown values retain offset-based names.
No live game session was used to infer the coordinate units, actor type meaning,
or the high-level effect of the container/UI callbacks.

**Source unit tests:** `test_emitter_update` passes on x64 and x86. Cases include
one-time/suppressed initialization, replacing elapsed on model completion,
signed coordinate limits, signed remaining-count rejection, elapsed excess,
at most one emission, exact-byte direction alternation, callback ordering,
counter wrap, callback replacement of the actor handle, signed zero, infinity,
and NaN emission. The binary64 countdown expectation is compared as bits to
avoid the test itself introducing x87 excess-precision comparisons.

**Offline native comparison:** `native_emitter_oracle` runs the unchanged
728-byte function at its original address in an owned 32-bit process. Only
the declared call boundaries are redirected. Its deterministic 20,000 cases
produce 4,021 emissions. The portable function matches callback arguments,
call order, callback-time countdown/flags/count/counter snapshots, final state,
and return low byte. Cases cover arbitrary floating point bit patterns,
subnormals, infinities, positive/negative quiet and signaling NaNs, negative
and zero intervals/elapsed values, raw non-boolean flag bytes, signed inputs,
both list branches, actor type changes during its update, and replacement of
the local actor pointer. Both unoptimized and `-O2` native comparison builds
passed all 20,000 cases, including exact timer and callback-double bits.

This validates the recovered control and arithmetic against native bytes with
adapters. It does not run the original callee implementations or a complete
game. The tested contract requires masked floating point exceptions, successful
allocation/construction, valid objects, and in-range counter indices. The
original has no graceful recovery when those object/index assumptions fail;
crash/exception-unwind behavior is not reproduced here.

Example invocation after building the 32-bit oracle:

```powershell
python tools/run_native_emitter_oracle.py `
  --exe 'C:\progs\Lemmings Revolution\Lemmings Revolution.exe' `
  --program build/x86/native_emitter_oracle.exe
```
