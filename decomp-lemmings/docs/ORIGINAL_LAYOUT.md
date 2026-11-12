# Original source layout

Target: installed reference `Lemmings Revolution.exe`, SHA-256
`32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f`,
image base `0x00400000`.

The retail build was compiled with assertions that expand `__FILE__`, so the
original build machine's source paths survive verbatim in `.data`. These are
the only original identifiers recovered from the executable so far: the build
carries **no RTTI type descriptors** (a scan for `.?AV...@@` returns zero
matches), so no class names are available from the binary.

## Recovered tree

| Virtual address | Path literal |
| --- | --- |
| `0x004c22a8` | `D:\lrpc\Source\3DEngine\HardwarePC.cpp` |
| `0x004c2574` | `D:\lrpc\Source\3DEngine\RawBitmap.cpp` |
| `0x004c2520` | `D:\lrpc\Source\3DEngine\TexturePC.cpp` |
| `0x004c284c` | `D:\lrpc\Source\File\FilePC.cpp` |
| `0x004c028c` | `D:\lrpc\Source\SkyIslands\LoadBMPImage.cpp` |
| `0x004c0a2c` | `D:\lrpc\Source\SkyIslands\SkyIslands.cpp` |
| `0x004c4470` | `D:\lrpc\Source\Sound\SoundManagerPC.cpp` |
| `0x004c33d4` | `D:\lrpc\Source\Sprites\SpriteSheets.cpp` |
| `0x004bfed8` | `..\Integral\Resources.hpp` |

So the original project rooted at `D:\lrpc` ("Lemmings Revolution PC") with at
least these directories:

```
D:\lrpc\
  Integral\          shared library, sibling to Source
    Resources.hpp
  Source\
    3DEngine\        HardwarePC.cpp  RawBitmap.cpp  TexturePC.cpp
    File\            FilePC.cpp
    SkyIslands\      SkyIslands.cpp  LoadBMPImage.cpp
    Sound\           SoundManagerPC.cpp
    Sprites\         SpriteSheets.cpp
```

Only these nine translation units assert, so the tree is certainly larger than
what is listed. Directory names are exact; absent files are not evidence that a
directory holds nothing else.

## What the names establish

**`SkyIslands` is the game module.** The world/level subsystem this repository
calls "world" was named SkyIslands upstream, matching the game's cylindrical
level towers. `SkyIslands started %d` (`0x004c0335`) sits beside the module's
own assert path.

**`Integral` is a separate shared library.** `..\Integral\Resources.hpp`
resolves to `D:\lrpc\Integral`, outside the game's `Source` tree — an in-house
engine or middleware layer rather than game code. Its `Resources not freed!`
string (`0x004bfef4`) is the same resource-manager subsystem already recovered
in `src/resource_*.cpp`, which means that work belongs to the shared library,
not to Lemmings Revolution specifically.

**The `*PC` suffix marks a platform split.** `FilePC`, `HardwarePC`,
`TexturePC` and `SoundManagerPC` all carry it while `RawBitmap`, `SkyIslands`,
`SpriteSheets` and `LoadBMPImage` do not. The consistent reading is a portable
core with per-platform implementations behind it; the executable alone does not
prove which other platforms existed.

## Asset path formats

Format strings naming on-disk and in-archive resources, useful when resolving
names recovered from compiled levels:

| Virtual address | Literal |
| --- | --- |
| `0x004c011c` | `Textures\%s.bmp` |
| `0x004c012c` | `Textures\%s.alf` |
| `0x004c33c4` | `Sprites\%s.spr` |
| `0x004c0368` | `Models\%s` |
| `0x004bfec4` | `Structure\%s.tga` |
| `0x004bff0c` | `Cylinder\%s\%s_%d_%d` |
| `0x004c3484` | `Language\%s.ltx` |
| `0x004c0324` | `Compiled\%s%s` |
| `0x004c0260` | `Thumbs\THM_%s` |

## Two leads worth following

**A software rasteriser exists.** `HardwarePC.cpp` carries
`No suitable 3D accelerator detected` (`0x004c2284`), four numbered
`Out of VRAM - Switching to software mode (N)` strings from `0x004c2310`, and
`Could not create surface in Video Memory - Switching to software mode`
(`0x004c2470`). A software path that needs no 3D accelerator is the natural
target for a standalone build, because it removes the Direct3D immediate-mode
dependency entirely. Only the strings are confirmed here; the fallback code has
not been traced.

**There may be a usable debug log.** `c:\Lemmings.log` (`0x004c0314`) sits
among trace-shaped format strings: `SkyIslands started %d` (`0x004c0335`),
`event missed by %dms` (`0x004bff50`), and
`Attempting to load background bitmap %s` (`0x004c451c`). If that logging is
still reachable in the retail build it would supply a runtime trace oracle at
no cost. **Reachability is unverified** — the strings being present does not
prove the writing code survives or can be switched on.
