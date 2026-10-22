# B1f: three image-matching passes, partial work and rejected glyph inference

Packet: `build/tmp/b1f-menu-template-match/`, first note, repair1-note.md and
repair2-note.md. Scripts: `tools/harness/measure_b1_menu_layout.py` and its test.
All three dispatches were offline, zero stock starts/REA/install writes.

## Retained partial result

The script compares the retained reference against retail MAIN_LOGO.BMP. Its only
in-frame native-size translation is (0,0), with full-frame numerator5651991 over
235008000, score0.02405020680147059. This is a background-only discrepancy, including
the omitted UI, not proof of a perfect backdrop or unique resource choice.

The first row detector fragmented text-cut bars and omitted the short row. Repair1
returns five candidate red/orange support bands and adds source alpha components.
These are **colour-support bounds**, not an independently established native panel
rectangle/border convention. Neither source-component counts nor those rectangles
identify a font or glyph metric.

The latest diagnostic frame is retail backdrop plus flat panel primitives, with no
text rendered: numerator2625270 /235008000 =0.011170981413398692. **It is not a B1
pass**, despite its scalar value. No production menu renderer exists. The user asked
for actual sprite/glyph matching and rendering, not a low-error omission surrogate.

## Rejected glyph inference and facilitator check

Repair2 finally executes a search, but fixes source cells to16x16, row-major ASCII32
without establishing that geometry from the atlas. It fits unique character identities
independently to target components, allows reuse of the same target component and does
not establish character order/advances. The poor Quit score therefore cannot reject
the retail font families, and no glyph layout is accepted.

More importantly, glyph_search resizes each scaled source silhouette AGAIN to the
target component's dimensions before comparison. The facilitator's bounded synthetic
check (`build/tmp/b1f-facilitator/search_checks.py`) demonstrates scale1 and scale2
both returning zero error and identical source/destination rectangles, although the
scale2 geometry is incompatible with the reported destination. Thus the reported
fractional-scale failure is not a valid discriminator of physical scale. All7 existing
tests pass despite this problem; they do not protect the missing registration behavior.

Independent RGB/RGBA comparison also corrects the packet's explanation of the two
`_FONT` families tying: full RGBA pixels differ, while their alpha planes are identical.
An alpha-only metric cannot distinguish them. The independent full-frame integer
checker reproduced the earlier background and omission-surrogate numbers; none is
promoted to production acceptance. No actual retail glyph reconstruction/placement
mutation is claimed. The exact source/template/reference files remain unchanged.
Fresh wrapper rebuild had no work; full CTest29/29 and all7 Pillow tests pass. These
green suites do not discriminate the invalid scale registration, as the independent
control demonstrates.

## Cap and the bounded exception

This fixed-cell/independent-component method is parked after3 dispatches, not silently
continued. B1h is an explicit **fourth-dispatch exception on the same glyph-layout
question**, not a cap reset: now there are two concrete falsifiable defects (unsupported
source packing and size-normalizing comparison), whereas the first two passes had
attempted no glyph search at all. A small atlas-independent search preserving actual
scaled geometry, beginning with the four-letter Quit row, is expected to move the
gate by providing real source glyph rectangles rather than another component count.
It must show a wrong-scale/placement control and may report an actual scored negative.
No fifth matching pass is automatic; no broad native draw-chain trace is substituted.

The source tool is retained as an explicitly diagnostic/failed-method reproduction,
not a production layout generator. Resume authority is the B1h brief/queue; B1g's
production renderer waits for useful independently verified glyph-layout evidence.
