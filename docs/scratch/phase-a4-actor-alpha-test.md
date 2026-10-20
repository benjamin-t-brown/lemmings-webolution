# A4u: actor alpha test implemented; pixel-invisible on the A4f fixtures

Queue `a4u-actor-alpha-test`. Offline code round, no game process. Worker packet:
`build/tmp/a4u-actor-alpha-test/round-note.md` (prediction `prediction.md`, written
before the gate rerun; histogram `alpha-histogram.txt`; gates `gate-new/`,
`gate-no-alpha-test/`; final sources and hashes `final/`, `final.sha256`).
The first worker died on a model-gateway error after writing the prediction and
histogram, with no source change. A continuation on the same pin finished the work.

## Result

- **Code:** `actorSpriteAlphaTestPasses(a)` = `a > kActorAlphaTestReference` (`0xA0`)
  in `src/assets/LightMap.{h,cpp}`. `drawActorSprite(..., alphaTest = true)` skips a
  failing texel, so the destination is untouched. A passing texel is written as
  `MODULATE` RGB with overlay alpha 255. Under the harness's `alpha_composite` that is a
  pure replace, which models selector 0 (blend disabled). `--no-alpha-test` on
  `actorfixture` and `verify_actor_sprite_a4f.py` restores the A4q keep-alpha write.
- **MEASURED (offline):** both fixture source frames are strictly binary alpha:
  - five, on `$LEMSHEETHI13`: 492 texels at 0 and 496 at 255;
  - eight, on `$LEMSHEETHI12`: 586 at 0 and 554 at 255.
  The test therefore selects exactly the texels the colour key already selected.
- **Gate:** unchanged to the last digit.
  - five: 0.0032284 against absent-actor 0.0061900;
  - eight: 0.0036200 against 0.0068484;
  - support MAE 0.0819 / 0.0778.
  The composed PNG hashes are identical with and without the test. Only the overlay
  differs: rejected texels are now `(0,0,0,0)`.
  **The change is pixel-invisible on these fixtures**, as the worker predicted.

## Scope limits

- The five fixture's frame is on `$LEMSHEETHI13`. A4t measured flags `0x2` only on
  `$LemSheetHi15` and `$LemSheetHi12`, so row 3 for Hi13 is **assumed, not measured**.
- This models the encoded flags-cache-miss path, not effective device state.
- The real sheets cannot tell `>` from `>=` or a nearby reference value. Only the
  synthetic unit test discriminates those.
- The ~0.08 support residual is untouched. Actors are still not drawn in the app.

## Facilitator check (2026-10-19)

- **Reread original bytes:**
  - `0x47283F 83e103` (`and ecx,3`), then `je 0x4728CA`;
  - `0x472880..0x472893`: state `0xF`, value 1, via vtable `+0x58`;
  - `0x472896 8b15e0244c00`: the reference load from `[0x4C24E0]`, pushed with state `0x18`;
  - `0x4728B0..0x4728C2`: state `0x19`, value 5.
  In D3D numbering those are ALPHATESTENABLE, ALPHAREF and ALPHAFUNC GREATER. The names
  are an SDK-numbering inference, as in A4r.
  - `0x4C24E0` in file `.data` is `a0 00 00 00`, inside rawsize.
- **Fresh build:** all owned sources touched first, then 20/20 CTest.
- **Own gate rerun** (`build/tmp/a4u-actor-alpha-test/facilitator/gate/`): the worker's
  MAE, composed and overlay hashes are reproduced exactly. The five composed hash equals
  A4q's facilitator gate.
- **Own isolated mutations**, each restored with hash match and fresh mtime; log
  `facilitator/check.txt`, `check2.txt`:
  - `>` → `>=`: `test_actorsprite` fails, "alpha 0xA0 is rejected (strictly greater)";
  - removing the reject `continue`: `test_actorsprite` fails, "$ gray-key alpha remains
    transparent". This is a behavioural failure from the pre-existing key test, since a
    rejected texel would now be written opaque.
  - After restore: 20/20.
- **Narrowed:** the worker's reference-value mutations (`0xA1`, `0x9F`) tripped the
  literal-constant assert first, so they were not shown failing behaviourally.
- **Live side:** no Lemmings process and no `LemWeb-*` task; the round never started
  the game.
