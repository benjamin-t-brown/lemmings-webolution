# Actor direction, sampling and terrain contact

Reference identity and mapped byte SHA-256 values are pinned in
`tools/verify_reference.py`. The native runner checks that identity and every
body and constant before passing bytes to its owned x86 process.

| Address | Bytes | Role |
| --- | ---: | --- |
| `0x0042F66B` | 129 | Negate direction at +0x40, set both sample X offsets |
| `0x0042F5E3` | 136 | Negate direction at +0x44, set +0x5C and sample Y offsets |
| `0x0042F22E` | 149 | Position plus +0x360 sample pair, unchanged Z |
| `0x0042E8C7` | 76 | Exact terrain-value/category eligibility |
| `0x0042E913` | 293 | Sample, clamp/wrap, read collision cell and filter contact |
| `0x0042FAD7` | 136 | Inclusive XY square with half extent one; ignore Z |

The direction negations wrap at 32 bits, including INT_MIN. Only the exact
new values -1 and +1 select special offset branches. Terrain eligibility
compares the **whole value** against 0x200, 0x4000 and 0x2000. Contact first
rejects zero, bit 0x80 and bit 0x1000, then applies that eligibility test.
For direction -1, the sampled X is advanced by one unless the primary state
type is 4 or 5. Y is offset by the vertical direction before clamping.
The float conversion reproduces the low 32 bits of the original x87 signed
64-bit truncation, including indefinite conversion where reached.

The source keeps x87 intermediate precision for XY containment. Ghidra's
two-double point signature omits an unused third double: the original
`RET 0x18` confirms three stack arguments. Z is never compared.

## Evidence and checks

REA pseudocode records: `ev_b923a563be17e741a308e815842be2aee032a8cff5146d8aa37c5dc12e346c30`,
`ev_f39026e35582ad9603d7c88a523eadd0550fb744d639b5820c3dc25be35fc4d1`,
`ev_7229035983b75bd07b3cebaa92eb832e2288716316c9ef2d6fcd3a6dfb7d0c08`,
`ev_fcff7e96f54ab742be334756666501470ca69f344a3615101fc3213749cbcd6c`,
`ev_2829542cb96b10254b7e51e5801f5bd45bb218bc37b48932f1289b0dc28d1ea9`,
`ev_afb5edc56c5ba1a63585e18d418c06eed1c1baf033ed760fbae0915c059f9a8f`.
Instruction inspection determines stack cleanup and floating-point branches.

`native_actor_geometry_oracle` executes unchanged original bodies and helper
bodies without call adapters. It compares 20,000 reversals, 10,000 samples,
100,000 terrain filters, 10,000 contacts and 10,000 XY bounds with the source.
Cases cover signed extrema, exact/mixed terrain masks, primary types 4/5,
NaN/infinite point coordinates, and ignored Z. Unit checks cover readable
boundary examples independently of the generated cases.

The grid and actor memory are supplied by the harness; this does not run the
game. Contact requires valid positive grid dimensions and backing storage.
Its tested coordinates are finite and inside the terminating domain of the
original repeated X wrapping. No exceptional CRT or nonterminating wrapping
behavior is claimed. Floating-point comparisons use the harness x87 mode;
the complete game's FPU initialization remains pending.
