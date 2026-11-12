# Shared model update and blinking transition

Reference executable SHA-256:
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`.

## Recovered behavior

`0x00432383` calls model vtable slot `+0x2C` with elapsed time when
object `+0x44` is non-null. It then re-reads flag `+0x40`. If nonzero,
it calls the current model's slot `+0x44` and returns whether the low
byte of that result is zero. Otherwise it returns true through the base
`0x00423470`. It does not gate this behavior on enabled flag `+0x38`.

`0x0043416C` adds a blinking transition. For nonzero mode at `+0x50`,
it remembers whether the initial mode equals one, subtracts elapsed time
from the double at `+0x48`, and stores the rounded binary64 result.
A negative or unordered result resets remaining time and mode to zero.
Otherwise it divides the stored time by the binary64 constant 0.2 at
`0x004B1210`, converts to signed 64-bit with `0x0049C5AC`, and
inverts the remembered Boolean when the low bit is zero.

The Boolean selects either zero or model slot `+0x4C`'s returned value.
That value is passed to slot `+0x38`, then the shared update runs.
The exact meaning of those model values is not established; the source
does not label them as opacity or animation time.

`0x00434261` and `0x00434290` set mode one/two, store
`5.0 * 0.2` as the remaining double (rounding to 1.0), and call
`0x0041DC00` / `0x0041DC20` to set enabled to one/zero.

## Machine-code details

- `FCOMP; FNSTSW AX; TEST AH,1` takes the reset branch for NaN too.
- The timer division remains on the x87 stack before the integer conversion.
  Using binary64 division prematurely would change some boundaries:
  binary64 1.0 divided by binary64 0.2 in extended precision is below five.
- `0x0049C5AC` explicitly sets rounding toward zero before `FISTP qword`,
  then restores the control word. An invalid masked conversion produces
  integer-indefinite `INT64_MIN`, whose low bit is even. The source models
  that result without an undefined out-of-range C++ cast.
- Callback state may change during model advancement. The source re-reads
  the removal flag and model pointer, matching the original.

## Evidence

REA shared reader:
`ev_2459e23971fffab6c158aaa6152e98c7ae01b1a0e718e6ee3b63dbea3781e110`.

REA blinking reader:
`ev_ae07b91913682ceec5cc583997daf1d5f86ffa4c165756f144097baca7d5f302`.

REA setters:
`ev_fc464b6a918d7d07ac2aa0533eff399b4e6d1e1a5485d19f77949ba1c5e262fa`,
`ev_cf4ce5319354f7588ac819b0e083ea85aee629ad215a4e0463dae8da2f544159`.

All eight code ranges are pinned in `tools/verify_reference.py`.
The native runner additionally checks the mapped zero, 0.2 and 5.0 constants.

## Validation and limits

`native_model_update_oracle` executes the eight original function bodies
unchanged at their original addresses in an owned x86 process. Only the
model's virtual methods are supplied as controlled callbacks. It passes
20,000 state-and-callback-trace comparisons, including null models in the
shared path, all byte values for flags/results, pointer replacement and flag
mutation during advancement, mode zero/one/two/other, signed zero, infinities,
quiet NaNs, fractional boundaries, and out-of-range integer conversions.
The timer setters are exercised during that corpus.

Unit tests run on x64 and x86 and cover callback order, post-advance flag
changes, extended-precision quotient boundaries, negative/NaN expiry and
enable/disable state. Native observations use the test process's masked x87
exceptions and control word. Other floating-point environments and original
model implementations remain outside the claim.

The public callbacks identify the unrecovered model-method boundaries.
These routines do not implement rendering, animation resources or model
lifetime management.
