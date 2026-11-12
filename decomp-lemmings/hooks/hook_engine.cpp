#include "hook_engine.hpp"

#include "hook_log.hpp"

#include <windows.h>

#include <cstring>

namespace lrhook {
namespace {

constexpr std::size_t kTrampolineSize = 64;
constexpr std::size_t kJumpLength = 5;

bool readable(std::uintptr_t address, std::size_t length) {
    MEMORY_BASIC_INFORMATION info = {};
    if (VirtualQuery(reinterpret_cast<void*>(address), &info, sizeof(info)) == 0) {
        return false;
    }
    if (info.State != MEM_COMMIT) {
        return false;
    }
    const auto region_end =
        reinterpret_cast<std::uintptr_t>(info.BaseAddress) + info.RegionSize;
    return address + length <= region_end;
}

} // namespace

bool image_base_matches() {
    const auto base = reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr));
    if (base != kReferenceImageBase) {
        log_line("image base 0x%08lx does not match the recorded 0x%08lx; "
                 "no detour installed",
                 static_cast<unsigned long>(base),
                 static_cast<unsigned long>(kReferenceImageBase));
        return false;
    }
    return true;
}

bool verify_bytes(std::uintptr_t address, const std::uint8_t* expected,
                  std::size_t length) {
    if (!readable(address, length)) {
        log_line("cannot read %u bytes at 0x%08lx",
                 static_cast<unsigned>(length),
                 static_cast<unsigned long>(address));
        return false;
    }
    if (std::memcmp(reinterpret_cast<const void*>(address), expected, length) != 0) {
        log_line("prologue mismatch at 0x%08lx; this is not the reference build",
                 static_cast<unsigned long>(address));
        return false;
    }
    return true;
}

void* install_detour(std::uintptr_t target, const void* replacement,
                     std::size_t prologue_length) {
    if (prologue_length < kJumpLength ||
        prologue_length + kJumpLength > kTrampolineSize) {
        log_line("refusing detour at 0x%08lx: prologue length %u is out of range",
                 static_cast<unsigned long>(target),
                 static_cast<unsigned>(prologue_length));
        return nullptr;
    }

    auto* trampoline = static_cast<std::uint8_t*>(
        VirtualAlloc(nullptr, kTrampolineSize, MEM_COMMIT | MEM_RESERVE,
                     PAGE_EXECUTE_READWRITE));
    if (trampoline == nullptr) {
        log_line("trampoline allocation failed for 0x%08lx",
                 static_cast<unsigned long>(target));
        return nullptr;
    }

    std::memcpy(trampoline, reinterpret_cast<const void*>(target), prologue_length);
    trampoline[prologue_length] = 0xE9;
    const auto resume = static_cast<std::int32_t>(
        (target + prologue_length) -
        (reinterpret_cast<std::uintptr_t>(trampoline) + prologue_length + kJumpLength));
    std::memcpy(trampoline + prologue_length + 1, &resume, sizeof(resume));

    DWORD previous = 0;
    if (VirtualProtect(reinterpret_cast<void*>(target), prologue_length,
                       PAGE_EXECUTE_READWRITE, &previous) == 0) {
        log_line("VirtualProtect failed for 0x%08lx (error %lu)",
                 static_cast<unsigned long>(target), GetLastError());
        VirtualFree(trampoline, 0, MEM_RELEASE);
        return nullptr;
    }

    auto* site = reinterpret_cast<std::uint8_t*>(target);
    const auto forward = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(replacement) - (target + kJumpLength));
    site[0] = 0xE9;
    std::memcpy(site + 1, &forward, sizeof(forward));
    // Pad the remainder of the relocated prologue so a disassembler reading the
    // patched image does not resynchronise mid-instruction.
    for (std::size_t i = kJumpLength; i < prologue_length; ++i) {
        site[i] = 0x90;
    }

    DWORD restored = 0;
    VirtualProtect(reinterpret_cast<void*>(target), prologue_length, previous,
                   &restored);
    FlushInstructionCache(GetCurrentProcess(), reinterpret_cast<void*>(target),
                          prologue_length);

    log_line("detour installed at 0x%08lx -> 0x%08lx (trampoline 0x%08lx, %u bytes relocated)",
             static_cast<unsigned long>(target),
             static_cast<unsigned long>(reinterpret_cast<std::uintptr_t>(replacement)),
             static_cast<unsigned long>(reinterpret_cast<std::uintptr_t>(trampoline)),
             static_cast<unsigned>(prologue_length));
    return trampoline;
}

} // namespace lrhook
