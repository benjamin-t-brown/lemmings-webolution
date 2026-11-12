# Lemmings Revolution source recovery

Recover the installed Windows `Lemmings Revolution.exe` into buildable C++.
The original executable is the authority. The sibling `lemmings-webolution`
project can supply context but is not evidence of what the original does.

**The goal is a working executable built from this source, not a byte-identical
rebuild.** Matching the original compiler's output is explicitly not a target.
Where readable C++ and byte-matchable C++ disagree, choose readable.

## How behaviour is established

Recovered routines run inside the original game through the harness in `hooks/`
(see [the hybrid runtime notes](docs/HYBRID_RUNTIME.md)). The running game is
the primary oracle: it supplies real inputs for free, and `shadow` mode
compares a recovered routine against the original bytes without being able to
affect play.

Use decompiler output as a **draft**, not as the deliverable. Read the original
machine code when the decompiler cannot be trusted:

- floating point, which is x87 throughout and where precision and control-word
  behaviour matter;
- unions, type punning, and any place a field is read at two widths;
- calling convention and argument pop, which must be exact for a hook;
- struct field offsets, which must be exact for a hook;
- anything where the decompiler's output is visibly garbled or implausible.

Outside those cases, do not hand-disassemble a routine before porting it. That
rule was the throughput bottleneck and it has been lifted deliberately.

## Evidence density

One ledger line per function in [the ledger](docs/FUNCTIONS.md): address, role,
source file, status. Write a prose evidence note only when behaviour had to be
*inferred* rather than read — a note should record reasoning, not restate what
the code already says. Per-function byte digests are no longer required; the
single executable SHA-256 identifies the build.

State what was actually verified. "Compared against the original in-process
over N cases" and "ported from decompiler output, not yet exercised" are
different claims and must not be written the same way.

## Non-negotiable

These are cheap and they are what keeps the hybrid from corrupting the game.

1. **Verify the executable identity** before making a claim about its code.
2. **Keep `static_assert(offsetof(...) == ...)` on every recovered struct.**
   Hooked code reads the game's own objects; a wrong offset is a crash, not a
   wrong answer.
3. **Keep unknown fields explicit.** `std::array<std::byte, N> unknown_prefix`
   is correct and must not be filled in from the remake or from a guess.
4. **Respect the allocator boundary.** Memory the game frees must come from the
   game's own `operator new`. Never free game memory with the host CRT.
5. **Do not store original game binaries or assets in this repository.** Read
   the user-owned installation in place.
