# Animation playback and model wrappers

Reference executable identity and all original bodies are pinned in
`tools/verify_reference.py`. The runner additionally checks constants at
`0x004B1940` (binary64 0.06) and `0x004B1928` (binary32 1e-8 widened to double).

| Address | Bytes | Recovered operation |
| --- | ---: | --- |
| `0x00468833` | 645 | Advance frames and update completion event |
| `0x00468AB8` | 76 | Select named clip and reset frame/fraction |
| `0x00468B04` | 89 | Total clip duration |
| `0x00468B5D` | 70 | Current elapsed animation time |
| `0x00468BA3`, `0x00468BD9` | 20, 25 | Read finished / set looping byte |
| `0x0046836D` | 88 | Release old resource ID, acquire replacement |
| `0x004840F3` | 88 | First exact clip-name match, count if absent |
| `0x00468D00`, `0x0041A4E0` | 30 each | 0x98-byte clip array indexing |
| `0x0045B8F0`, `0x00484150` | 16 each | Frame/clip count access |

The inline actor model constructed by `0x0043CE00` uses vtable `0x004B14A8`.
Its playback state starts at model +0xB0. Slot +0x2C (`0x0043CF30`) advances,
+0x30 (`0x0043CF53`) selects by name, +0x34 (`0x0043CF72`) selects by index,
+0x38 (`0x0043D030`) sets the fraction, +0x3C (`0x0043CFC8`) sets looping,
+0x44 (`0x0043CFAF`) reads completion, +0x48 (`0x0043CF17`) reads elapsed time,
and +0x4C (`0x0043CEFE`) reads duration. All wrappers execute in the native check.

Only direction exactly +1 advances forward; every other integer reverses.
Each update adds `elapsed * clip.speed` to the stored fraction, clears the
finished byte, and steps while fraction is strictly greater than 0.06.
Forward playback detects the endpoint by exact frame-count equality.
Reverse playback detects a negative index. Nonzero looping wraps; zero
looping clamps. Crossing an endpoint sets finished even when looping.
The flag is cleared on the next update, including an update of zero duration.
Forward residual rounding clears values <= epsilon; reverse clears values
strictly < epsilon. Integer frame arithmetic wraps at 32 bits.

Each loop subtracts the binary64 0.06 separately. Replacing the loop with a
division changes boundary behavior. Intermediate products and sums retain
x87 precision until their observed stores. Time queries return the final
division in extended precision. Name/index selection does not clear finished.
Duplicate names choose the first match, case sensitively. Missing names
produce a one-past-end index in the original; the port preserves that result
but guards later invalid resource accesses with an exception.

REA core records: `ev_60b9c444c27cd07826b1a8c030b9e7f8ceddc7f72360c5a6d957b5d4dbd2a0e7`,
`ev_5950717e303f215075bb761b67ec6b2524861e6ce5a9a01038a8ca4623234bea`,
`ev_a08d8c984e08278bce9143cc6b9b1532acabec8064037cc7e7815fd4446034c3`,
`ev_4c584a3865d50a40f4662165535fbd18b39f6e7cfba14fa16589c5c33aac6cc4`,
`ev_ec063a53fced33714df6078cb6c47e31199c1b44c4a56e14ce68c151e33a316f`,
`ev_eb00c7f166e8788237e81af8b84427186743439d96ce5c19b8f37e7dcc3e0a14`.
Binding/name records: `ev_6a88fc9995707a7fedffa4ab576e2abd37bef6ae36ab54f670f6f1740465b8cd`,
`ev_56a1abbdd551d3829b8dde1af313af8126c3cde9e5ee685f85862a797346f2ee`.

## Checks and boundaries

Readable unit cases run on x64 and x86. The native oracle compares 30,000
advances, time/duration/finished query sets, name selections, index/fraction/
loop setters, and sampled resource rebindings at both O0 and O2. It checks
stored fields, extended query values and resource-call traces. NaN payloads
are compared by class, finite stored values by bits. The original routines,
wrappers, array helpers and CRT strcmp execute unchanged; only global
resource-cache acquire/release are explicit adapters.

An initial generated case combined negative infinity with negative speed,
producing positive infinite accumulated time; the original loop did not
terminate. The runner timed out and that out-of-domain case was removed.
Positive infinity and finite values too large for subtraction to progress
remain outside the terminating input contract. The native harness does not
validate resource loading, actual frame rendering, exceptional process FPU
settings or full game execution. Clip frame payloads are still unrecovered.
