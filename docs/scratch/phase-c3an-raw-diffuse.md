# C3an initial/repair1 — actual raw diffuse adoption

2026-10-25. Packet `build/tmp/c3an-raw-diffuse/repair1-note.md`; original note,
reports and failed coverage preserved under `before-repair1/` (33 files,
manifest independently authenticated). No stock start/attach/native call.

## Facilitator check

Read every changed source/test/CMake hunk. `packSceneDiffuseWord` is shared by
the existing byte packer and `adaptSceneMeshVertex`, which actual BackdropScene
calls with raw source attributes. It truncates raw signed32 channels and performs
unsigned shifts/OR before ARGB decoding; no normalization, individual byte clamp
or premature per-channel mask. The signed32 rejection domain is software scope,
not native overflow proof. Geometry, UV, RHW, predicate, AL and AM unchanged.

Fresh bytes46D4F6..D5B9 and49C5AC..D2 re-read: truncate calls feed alpha<<24,
R<<16,G<<8,B then OR; ftol selects chop and returns qword low32. Logs under
`build/tmp/c3an-independent/`. Source-level compatibility, not effective device
state or native overflow/precision acceptance.

Initial worker mutation bundled rescale and independent byte casts. Own **rescale
alone** before shared word packer survived initial permanent test: negative blue
made its only adapter/pixel wordFFFFFFFF, masking rescale. Returned for bounded
repair, preserving the passing mutant and initial wrongOR test expectation.
Repaired zero/one/fraction/signed-spill destinationRGBA tests expose diffuse alpha.

Own independently generated **6561 rawword/adapter/pixelRGBA cases** pass. Own
rescale-only source mutant now fails both adapter and destinationRGBA permanent
one/fraction cases; independent oracle also fails. Exact source restoration
c736ec0a8c08b87daaddc27a8a04f46b9a940af03b88c440fc96e7962f3b4972;
restored full build and **43/43 CTest** pass. Existing2077retained+2077synthetic
known-byte fixtures still pass (including preserved specular), independent old
billboard17/340replay unchanged. Scripts `build/tmp/c3an-independent*`.

## Fresh fixed gate and honest pixel limit

Own fresh `build/tmp/c3-check-an-final/` repair4/all10C1 replay and independent
histogram agree with explicit-reference scorer:
**14837570/235008000=0.0631364464 FAIL**. Frame SHA256
409fbdcb1ee5c94abd705745de14eb8b4afbb91c999072f462de8c450109c956.
Rest/floor12401492,skills1495489,tube674411,counters175496,caption75548,
historical actor boxes15134. Floor remains included. No gate/mask/reference change.

The sole rescale mutant renders the prior AM frame14829164: **8406 lower** error.
Thus the raw value is protected by source/independent numeric/pixel-unit checks,
NOT by a worsening original-screenshot perturbation. Keep the accepted raw
contract despite worsened score; no score-selected normalization or native pixel
fidelity claim. Own fresh scene-off remains exact baseline
13b1bd501803c3bf3e210cac651b1968083b26c74a852925e04e5e6f44e3829e.

C1 footprint numerators /17721990: zoom1285083,elev_down285700 (gated passes),
elev_up280461,initial1499220,canonical1499220,elev0_neg1498840,mouse1703302,
right1726955,left1415505,MM_DIG_0001795145. Supplemental full-frame scores retained.
Three install hashes match pre-run; separate process/task check empty. No live
resource. Dispatch20:20:03,repair20:32:42; exact landing/elapsed in archive.
Next sorted secondary mixed dispatch; type1/clip/finalf32/state remain open.
