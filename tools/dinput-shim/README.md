# dinput.dll shim

Lets a script drive the original Lemmings Revolution. See PLAN.md §2.1 for why
this is necessary and how it works.

## Build

```sh
./build.sh          # needs mingw-w64-i686-gcc (32-bit; the game is a 32-bit process)
```

## Install

Copy `dinput.dll` into the Lemmings Revolution folder, next to the game exe.
`dinput.dll` is not a KnownDLL, so the application directory wins the DLL search
and the game loads this instead of the system one. The shim then loads the real
`SysWOW64\dinput.dll` itself and forwards to it.

**It is inert until a harness enables it.** With `enable = 0` in the shared block
(the default) every call passes straight through to the real device, so the game
plays exactly as normal. To uninstall, just delete the file.

## Use

```python
from lrinput import LRInput, DIK

with LRInput() as inp:
    inp.enable()             # take over
    inp.tap(DIK['DOWN'])     # one keypress, frame-accurate
    inp.move(0, 25)          # one tick of relative mouse motion
    inp.click()              # button 0 down/up
    inp.release()            # hand input back to the physical devices
```

`python lrinput.py` on its own prints whether the shim is live and how fast the
game is polling it.
