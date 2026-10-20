# REA — verified setup on this machine

[REA](https://github.com/morluto/rea) is the Ghidra-backed decompilation tool this project
now uses as its primary static reader. Set up and verified working against the stock exe on
2026-10-18.

## The recipe that works

```powershell
npm install --global rea-agents          # 6.5.0 or newer

$env:GHIDRA_INSTALL_DIR  = 'C:\progs\ghidra\ghidra_12.1.4_PUBLIC'
$env:JAVA_HOME           = 'C:\Program Files\Java\jdk-24'
$env:REA_ANALYSIS_PROVIDER = 'ghidra'
$env:REA_GHIDRA_LANGUAGE_ID = 'x86:LE:32:default'   # our target is a 32-bit PE

rea doctor
rea inspect "C:\progs\Lemmings Revolution\Lemmings Revolution.exe" --provider ghidra
```

Verified result: `architecture: x86`, `format: pe`, digest matching the authority's SHA-256
in `docs/LESSONS.md`, and `import_mode: ephemeral-source-immutable` — REA analyses an immutable
copy and never writes to the game install.

**Correction:** the previous abbreviated digest in this paragraph had an incorrect suffix.
The facilitator independently hashed the installed stock executable and confirmed the full
digest already recorded in `docs/LESSONS.md`; do not reuse the retired abbreviation.

## Codex connection is separate from CLI installation

**Setup correction:** the npm package and Ghidra provider were installed, but Codex had no
REA MCP registration. Installing the CLI or skill alone does not connect an existing client.
With the provider environment above set, use the installed package's scoped setup:

```powershell
& "$env:APPDATA\npm\rea.cmd" setup --client codex --dry-run --json
& "$env:APPDATA\npm\rea.cmd" setup --client codex --yes --json
& "$env:APPDATA\npm\rea.cmd" doctor --client codex --json
```

Inspect the dry-run before applying it. The applied setup added only `mcp_servers.rea` to
`C:\Users\bbrown\.codex\config.toml` and refreshed the installed REA skill. The previous
configuration is backed up as `config.toml.rea.backup`; an additional configuration and
skill backup is under `C:\Users\bbrown\.agents\skill-backups\`. A parsed before/after
comparison confirmed unrelated Codex settings were preserved.

**Facilitator check:** the scoped doctor reported healthy, a separate server completed MCP
initialization and a session-status call, and after client refresh the actual Codex session's
`binary_session` call succeeded with Ghidra available. These establish the connection, not a
camera measurement. Refresh the client when registration changes, then check the actual
session's tools rather than inferring a connection from a successful CLI command.

**Cursor (2026-10-19):** `~/.cursor/mcp.json` had a `rea` entry pointing into a temporary
`npm-cache\_npx\…` copy. Cursor reported the server as failing discovery, and `rea doctor
--client cursor` marked it `stale`. The fix is the same scoped setup with `--client cursor`
(dry run first). It rewrites only the `rea` entry to the global
`%APPDATA%\npm\node_modules\rea-agents\scripts\rea.mjs`, keeps `GHIDRA_INSTALL_DIR` /
`JAVA_HOME`, and backs up to `mcp.json.rea.backup`. Doctor then reports `aligned`.
Restart Cursor afterwards. The registered Ghidra is `C:\tools\ghidra_12.1.4_PUBLIC`.
`C:\progs\ghidra\…` also exists; doctor checks the registered one.

The Codex sandbox can hide the user's npm directory and Python user-site packages. In this
session, Capstone and Pillow were already installed but appeared missing to sandboxed
Python; running the same interpreter with authorized normal-user access found them. The
ordinary test command also passed with that access after a sandbox-only progress save/load
failure. Diagnose access before reinstalling packages or changing source to accommodate it.

## Two traps that cost time, so you don't repeat them

**1. Do not run the source clone.** There are clones at `C:\progs\rea` and `.rea`, both on
repository main. `npm install` and `npm run build:cached` both succeed there, but
`rea inspect` then fails with `runtime_missing`:

```
REA Windows native controls are unavailable: ENOENT:
  open 'C:\progs\rea\native\windows\build\manifest.json'
```

That addon **cannot be built on this machine**. `native/windows/README.md` says the artifact
lane requires **Linux** plus MinGW-w64 cross-compilers, and `build/` is gitignored. The
prebuilt addon ships only inside the published npm package. So: install from npm, keep the
clones for reading source and docs.

**2. `install.sh` refuses Windows outright** — it gates on `uname -s` being `Darwin` or
`Linux` and fails with "REA supports macOS and selected 64-bit Linux distributions." That is
the *installer*, not the tool. Use `npm install --global rea-agents` and the Windows Ghidra
path in `docs/windows-ghidra-p0.md`, which is supported (experimental) on Windows 10+ x64.

A third, milder trap: `rea doctor` reports `healthy: false` and exits non-zero even when
everything we need is green, because `hopper` is missing. Hopper is the macOS alternative
engine. Read the individual `ghidra-*` checks instead — all eight should say `healthy`.

## Prerequisites, as actually satisfied here

| Requirement | On this machine |
| --- | --- |
| Windows 10+ x64 | Windows 11 |
| Node 22.19+ / 24.11+ / 26+ | 24.13.1 |
| Ghidra 12.1.x (12.1.4 verified) | `C:\progs\ghidra\ghidra_12.1.4_PUBLIC` |
| 64-bit full JDK 21+ | JDK 24.0.2 at `C:\Program Files\Java\jdk-24` |
| native non-managed x86 or x86-64 PE | the game is a 32-bit PE |

REA does not install Ghidra, Java, Node or a compiler, and never needs PyGhidra.

`JAVA_HOME` is **not** set system-wide here — set it per shell as above, or set it
permanently if you prefer.

## What it gives us, and the boundary

A read-only set of inventory, memory and function-analysis operations: function dossiers,
decompilation of a named function, call relationships, string inventory, byte reads, load
image observations. That is exactly what the rendering and frontend work needs, because a
projection matrix or a menu layout cannot be measured by watching lemmings walk.

It does **not** give Ghidra GUI state or annotations, and the Windows boundary is read-only.

## How to use it without breaking the one rule

REA output is a **reference**, not a result. The standard in `docs/LESSONS.md` still holds:
cite the address and the bytes. Two specific disciplines:

- **Do not paste decompiler output into the repository.** Our implementation stays
  clean-room: read what the original does, then write our own code. Verbatim generated
  source would make the result a derivative of the binary rather than a reimplementation,
  and it does not read or compile well anyway.
- **A decompiled line is still not a measurement.** Where behaviour can be observed live,
  observe it. Where it cannot — matrices, texture addressing, screen layout — say plainly in
  the note that the finding is READ, not MEASURED, and name the function.

Our own `tools/harness/read_steppers.py` (capstone) is still the quickest way to see the
actual bytes at an address, and the two tools complement each other: REA for "what does this
function do", `read_steppers.py` for "what are the bytes at `0x0041C154`".
