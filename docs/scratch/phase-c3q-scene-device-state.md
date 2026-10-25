# C3q — conditional scene request/state machine

2026-10-25. READ + DERIVED; **not effective pause-device state**. No stock start,
attach, original target execution, install or production edit. Packet:
`build/tmp/c3q-scene-device-state/round-note.md`; numeric replay/table and exact
remaining inputs are in that directory. Reproduce with normal Python + Capstone:
`python -B build/tmp/c3q-scene-device-state/replay.py`.

## Accepted branch/caching values

`0x47261F` compares pointer cache5CD5F4 first; a hit skips flags as well as binding.
On miss it stores the pointer and requests texture0, then compares the **full32-bit**
flags cache4C24E4. A flags hit skips material states. Cache writes precede API
requests; a failed call does not automatically invalidate/retry the cache.

On flags miss, every material path requests stage0 RGB `(OP,ARG1,ARG2)=(4,2,0)`
(MODULATE/TEXTURE/DIFFUSE). Ordered branches, with inherited values marked I:

| First condition | Alpha OP/ARG1/ARG2 | Alpha test | Ref/function |
| --- | --- | --- | --- |
| flags&8 |2/2/I|0|I/I|
| else flags&16 |4/2/0|0|I/I|
| else flags&3 |2/2/I|1|global4C24E0 /5(GREATER)|
| else flags&4 |4/2/0|0|I/I|
| else |2/2/I|0|I/I|

Anchors `0x47267D/272A/283F/28E5`, early exits271F/27E6, reference load2896 and
calls2880..28C2. SELECTARG1 does not reset ARG2; disabling alpha test does not
reset reference/function. C3r constructor flags0/4/8 can now supply this table,
conditional on its normal resource-construction route, not the file blend selector.

Blend routine `0x414D80` separately compares full selector cache4C227C:
selector0 requests disable and preserves factors;2 requests enable/ONE/ONE;
3 enable/SRCALPHA/ONE; other nonzero enable/SRCALPHA/INVSRCALPHA. A cache hit
preserves all three. Source mesh selectors are1095x0,82x1;17 type2 selectors1.

Null binder `0x472979` disables alpha test/binds null unless pointer cache is
alreadyFFFFFFFF; **both paths** set selector cache4 and flags cacheFFFFFFFF.
It does not reset stage/blend/filter/Z/cull/fog states. A following selector4
can retain an earlier additive blend; actual null-name meshes use0, forcing
blend-disable after that cache reset. This subtle cache behavior is preserved.

`0x470F29` requests POINT1/1 and writes global alpha source239 unless both its
argument's low8 bits and renderer+3A1 are nonzero, when it requests LINEAR2/2
and source160. It **does not request device ALPHAREF**. A later same-flags bind
can therefore draw LINEAR with a still-latched device reference239.
`0x470FAF` requests combined address WRAP1 for low8(arg)=0, CLAMP3 otherwise
(SDK combined-address semantics apply to U/V). No initial scene inheritance is
proved by the current frame's later tube restore.

Mesh wrapper entries set shade before their record loops: kind5/7 GOURAUD2
through `0x479F93/47A05D`, kind6 FLAT1 through `0x479FEA`. Bind/cache updates
at `0x469D71/469EAA` occur before kind5 predicate rejection, so a skipped record
does not undo them. Type2 inherits shade at its binder. Init helper `0x472170`
requests SPECULARENABLE0; call `0x444D0C` precedes the mode store444D14, but this
is not observed successful initialization on the paused device. Unwritten vertex
specular cannot be ignored unconditionally, especially with unknown fog behavior.

## Facilitator check

`python -B build/tmp/c3q-facilitator-check.py` authenticates all32 raw windows
against the PE and saves fresh disassembly. Complete material, blend, null/filter/
address/specular setters, shade helpers and bind/wrapper order were semantically
re-read. SDK checker55 enums/four slots passes against the same independently
hashed Device3 headers used by C3n/C3o; no installed wrapper state is inferred.

Separate finite branch expectations recompute **600** full80-field matrix rows,
including inherited symbols and exact cache values. Extra checks protect latched
alpha reference, null-selector4 blend preservation, in-place flag pointer hits,
and non-modifying rejected requests with no automatic retry. Source selectors
and304 null-name records independently recounted from accepted tables.
The packet has613 rows/2590 exhaustive cases; those larger totals are not claimed
as independently reproduced native observations. API-unknown alternatives and
unrepeated extra fixtures remain model data, not actual failure semantics.

Changing only private flags0->1 changes alpha-test enable/ref/function; restore
returns the full vector. Changing only the private decoded mask83E103->83E100
at47283F makes expected alpha-enable1 become0; a fresh original decode restores1
and original PE bytes remain83E103. No source/install/live mutation. Fresh build
and **34/34 CTest passed**; no Lemmings process or live owner remains.

## Still required

Unique effective ZENABLE/ZWRITE/ZFUNC/ZBIAS, CULLMODE, FOG and higher-stage/mip
state, accepted API effects, actual group order/resource identities and initial
cache state remain typed inputs (`input-requirements.json`). No GPU default or
transitive absence claim is accepted. C3 baseline stays0.2050523; no scene render.
Next obtain minimal exact state/format inputs (offline preflight first), alongside
independent pause-scene reachability. Dispatch14:16:53; worker21m26s; accepted
14:48:32 MDT, **31m39s** dispatch-to-acceptance including facilitator review.
