# Original-game session 1, 2026-11-14 16:31-16:34 (interactive, user playing)

What ran: `C:\Users\bbrown\Downloads\Lemmings Revolution\Lemmings Revolution.exe` (sha256
32675bdb49866e76..., identical to the repo's reference EXE), windowed through dgVoodoo
(`FullScreenMode = false`), all-unlocked save swapped in, no hooks or debugger. ffmpeg recorded
the 640x480 client area at 20 fps to `session-1.mkv` (3,697 frames, about 185 s; the file was
ended by killing the recorder, so players report "ended prematurely" but every frame decodes).
Backups of the install's previous `dgVoodoo.conf` and `SavedGame.dat`: `backup-downloads/`.

Timestamps below are video time. These are observations of the original, read by the lead from
contact sheets; pixel measurements have not been taken yet.

## Exit balloon departure (0:13.5-0:19), sheet `balloon-a.png`
- Last lemming is saved at about 0:13.5 (counter reaches 0). The balloon leaves its perch at
  about 0:14.0 and is off the right edge of the view by about 0:18.5; the map screen follows at
  about 0:19-0:20. So the departure lasts roughly 4.5-5 s before the level is left.
- With the camera still, the balloon moves sideways away from the cylinder at a roughly steady
  rate, with little height change over those seconds. It does not go straight up.
- Only one departure was recorded, from one camera angle, so the 3D path (how much is outward
  and how much is around the cylinder) is not determined by this alone.

## "Introducing - Water Lemmings" (0:25-1:25), sheets `pit-a.png`, `pit-zoom.png`
- At level start the top container is full and the two bottom containers are EMPTY. This
  agrees with the clone's data-derived initial state (fluid-kinds-001, lever-render-001).
- A lever on the middle ledge was thrown at about 0:46; the right bottom container fills over
  about 4 s (0:47-0:51) and the left one starts filling at about 0:51.
- After filling, water lemmings travel along the water surface (0:50 onward).
- NOT observed: a lemming reaching an empty bottom container before the lever was thrown.
  The empty-pit behavior is therefore still unobserved.
- Water lemmings are drawn with teal/cyan hair and a pale body, visibly different from the
  green-haired ordinary lemmings of the first level. Close-up frames not yet extracted.
- The attempt ended by Nuke with 8 of 10 saved ("You Failed To Rescue Enough Lemmings").

## Third level (1:45-3:00), sheet `balloon-b.png`
- 89 saved of 50 needed; the level ended when the clock reached 00:00 and went to the results
  screen ("You Totally Stormed That Level!"). No balloon departure was shown in this case.

## Map hover text
- One frame at about 1:40 shows the three-line block ("20 Lemmings : Save 10", "Bash and Mine",
  clock icon, "10:00") in the black rounded font with red shadow; usable as a second reference
  for map-hover-text-001 beside docs/playtest/reference/leveltext.png.

## Not covered in this session
- "Build and Block": an ordinary lemming entering water (drowning look).
- A climber with the camera rotated so its wall is in front of it.
- A lemming at an empty container without the lever thrown.

# Session 2, 2026-11-14 16:37-16:39 (same install, same setup)

`session-2-game.mkv`: 640x480, 20 fps, about 162 s. The recorder captured the whole desktop by
mistake (the window rectangle was not passed to ffmpeg); the lead cropped it to the game's client
area at (6,0) and deleted the full-desktop file. Timestamps are video time.

## Empty container without the lever (1:36-2:08), sheets `s2-pit-1.png`, `s2-pit-2.png`
- "Introducing - Water Lemmings", lever not thrown. Lemmings reaching a bottom container FALL
  INTO it: falling figures are visible inside the empty glass at 1:48, 1:49, 1:51, 1:52, 2:00
  and 2:01. None survives: the out counter goes 8 -> 0 with 0 saved and the level ends by
  itself at about 2:07 with "You Failed To Rescue Enough Lemmings! You Needed 10, You Saved 0".
- So an empty container gives no support, and the level cannot be won without the lever by
  walking across. Whether death is by fall height or by the container floor itself is not
  distinguishable from these frames.

## Drowning on "Build and Block" (2:32-2:38), sheet `s2-drown-zoom.png`
- An ordinary (green-haired) lemming walks off the ledge onto the water at about 2:34.25.
- 2:34.5: it flails at the surface, drawn brightened; 2:34.75: small splash droplets above it.
- 2:35.0-2:36.0: it sinks below the surface as a pale translucent figure that breaks up into
  bubbles while descending roughly one to two lemming heights; gone by about 2:36.25.
- Whole sequence about 2 s. The out counter drops by one between 2:36 and 2:38.

## Second balloon departure (1:23.5-1:28), sheet `s2-balloon.png`
- Same level and almost the same camera angle as session 1. Last lemming saved at about 1:23.5;
  the balloon leaves at about 1:24.0 and is out of view by about 1:28: about 4 s, sideways and
  away, matching session 1. Still a single viewing angle.

## Hover feedback seen in passing (0:24 and 2:24, `s2-sheet.png`, `s2-buildblock.png`)
- With the cursor on the hatch, the remaining count ("20") is drawn in white above the hatch.
- A hovered lemming shows green corner brackets with a letter beside it.
  Both are worth a closer look against the clone's provisional hover look; not examined further.

## Still not covered
- A climber with the camera rotated so its wall is in front of it.

# Where the evidence is
Reduced JPEG copies of the contact sheets named above are under
`docs/playtest/reference/original-2026-11-14/` (same base names). The recordings and full-size
PNG sheets are local only, under ignored `build/lead/original-session/`. These are observations
of the original by a person playing it in an interactive lead session; they are reference for
clone presentation and rules, not a fidelity measurement, and no queue worker ran the original.
