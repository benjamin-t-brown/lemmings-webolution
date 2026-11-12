#pragma once

#include <cstddef>
#include <cstdint>

// Entry-point detour installation for the reference executable.
//
// Recorded virtual addresses are only meaningful when the executable is mapped
// at its preferred base, so every install verifies the image base and the
// original prologue bytes before writing anything.

namespace lrhook {

// Preferred image base of the reference Lemmings Revolution.exe.
inline constexpr std::uintptr_t kReferenceImageBase = 0x00400000u;

// True when the main module is mapped at kReferenceImageBase.
bool image_base_matches();

// Compares `length` bytes at `address` with `expected`.
bool verify_bytes(std::uintptr_t address, const std::uint8_t* expected,
                  std::size_t length);

// Writes a five-byte relative jump to `replacement` at `target`, after copying
// `prologue_length` bytes into a freshly allocated trampoline that ends with a
// jump back to `target + prologue_length`.
//
// `prologue_length` must be at least 5 and must land on an instruction
// boundary. The relocated bytes must not contain relative branches and must
// not be the destination of any branch inside the function; both hold for the
// ordinary `push ebp / mov ebp,esp / push ecx / mov [ebp-4],ecx` prologue this
// harness targets.
//
// Returns the trampoline, which is callable with the original calling
// convention, or nullptr when installation failed.
void* install_detour(std::uintptr_t target, const void* replacement,
                     std::size_t prologue_length);

} // namespace lrhook
