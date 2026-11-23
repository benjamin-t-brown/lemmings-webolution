# Mobile phone request (2026-11-23)

User request for a phone pass on the web build; no play session. Clone additions, labelled
provisional where a threshold is chosen. Desktop mouse and keyboard behaviour stays as it is.

## Requests
- V1: enforce landscape on phones.
- V2: touch camera controls: one-finger drag rotates; pinch toggles the zoom in and out (the
  existing zoom toggle, today Space or Ctrl).
- V3: touch assignment. Touching a lemming assigns the selected skill at touch start. Touching and
  holding an empty spot gives a precision cursor that behaves like the mouse pointer (hover
  highlight) and can be dragged around.

## User decisions (2026-11-23)
- Precision cursor: lifting the finger assigns the selected skill to the highlighted lemming;
  lifting with nothing highlighted does nothing.
- Precision cursor position: drawn a short distance above the finger so the finger does not cover
  the target, moving with it.
- Hatch release rate on touch: touch and hold a hatch, then slide up to raise and down to lower,
  stepping at the mouse-hold speed while past a small threshold.

## Queued
web-landscape-001 (V1), touch-camera-001 (V2), touch-assign-001 (V3 and the hatch decision).
