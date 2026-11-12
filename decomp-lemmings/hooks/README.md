# In-process hook harness

Loads the recovered library into the running original `Lemmings Revolution.exe`
and replaces routines one at a time. Full rationale, phase plan, detour
mechanics and the four ABI hazards are in
[docs/HYBRID_RUNTIME.md](../docs/HYBRID_RUNTIME.md); the verification record is
in [docs/evidence/hook_harness.md](../docs/evidence/hook_harness.md).

| File | Role |
| --- | --- |
| `patch_proxy.cpp` | `DllMain`, plus `DirectDrawCreate` / `DirectDrawEnumerateA` forwarders to `patch_orig.dll` |
| `patch_proxy.def` | Exports those two names undecorated |
| `hook_engine.cpp` | Image-base and prologue checks, `JMP rel32` installation, trampoline construction |
| `hook_config.cpp` | `LEMMINGS_HOOK_MODE`: `shadow`, `verify`, `replace` |
| `hook_log.cpp` | `lemmings_hooks.log` in the installation directory |
| `hooks_collision_grid.cpp` | First hooked routine, `0x0040DE60`, with its startup sweep |
| `build_hooks.sh` | 32-bit MinGW build; links `build/x86/liblemmings_recovered.a` |
| `install.py` | Idempotent install, status and restore against a game directory |

```bash
bash hooks/build_hooks.sh
python hooks/install.py --install "C:/progs/Lemmings Revolution"
```

The harness is built by a script rather than CMake because it needs the 32-bit
MinGW toolchain specifically and produces a DLL destined for a game directory
rather than a test binary.

To add a routine, follow the checklist at the end of
[docs/HYBRID_RUNTIME.md](../docs/HYBRID_RUNTIME.md).
