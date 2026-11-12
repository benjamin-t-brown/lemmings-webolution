// Execute original entity-reader bytes with explicit non-stream adapters.
#include "lemmings/level_entity_prefix.hpp"

#include <windows.h>
#include <fcntl.h>
#include <io.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <random>
#include <vector>

namespace {
constexpr std::uintptr_t image_base = 0x00400000;
constexpr std::size_t image_size = 0xD0000;
using NativeRead = void (__attribute__((thiscall)) *)(void*, void*);
struct MemoryStream {
    const std::uint8_t* data;
    std::size_t size;
    std::size_t cursor;
};
std::array<std::uint8_t, 40> allocation{};
bool fail_allocation = false;
int allocations = 0, constructors = 0, resets = 0, terrain_calls = 0, attachments = 0;

std::uint32_t __attribute__((thiscall)) read_adapter(
    MemoryStream* stream, std::uint32_t count, void* destination) {
    const auto copied = std::min<std::size_t>(count, stream->size - stream->cursor);
    std::memcpy(destination, stream->data + stream->cursor, copied);
    stream->cursor += copied;
    return static_cast<std::uint32_t>(copied);
}
void* __cdecl allocation_adapter(std::uint32_t) {
    ++allocations;
    return fail_allocation ? nullptr : allocation.data();
}
void* __attribute__((thiscall)) constructor_adapter(void* object) {
    ++constructors;
    return object;
}
void __attribute__((thiscall)) reset_adapter(void*) { ++resets; }
void __attribute__((thiscall)) terrain_adapter(void*, void*) { ++terrain_calls; }
void __attribute__((thiscall)) attachment_adapter(void*, void*) { ++attachments; }

void write_jump(std::uintptr_t address, std::uintptr_t target) {
    auto* code = reinterpret_cast<std::uint8_t*>(address);
    code[0] = 0xE9;
    const auto displacement = static_cast<std::uint32_t>(target - address - 5);
    std::memcpy(code + 1, &displacement, 4);
}
template<std::size_t Size>
bool read_at(std::uintptr_t address) {
    std::array<char, Size> bytes{};
    std::cin.read(bytes.data(), bytes.size());
    if (std::cin.gcount() != static_cast<std::streamsize>(bytes.size())) return false;
    std::memcpy(reinterpret_cast<void*>(address), bytes.data(), bytes.size());
    return true;
}
template<class T>
void put(std::vector<std::uint8_t>& bytes, T value) {
    const auto old_size = bytes.size();
    bytes.resize(old_size + sizeof(value));
    std::memcpy(bytes.data() + old_size, &value, sizeof(value));
}
template<class T>
void store(std::array<std::uint8_t, 0x3A8>& bytes, std::size_t offset, T value) {
    std::memcpy(bytes.data() + offset, &value, sizeof(value));
}
// FLD/FSTP quiets a signaling binary64 NaN. The source model retains the
// serialized bits, so compare the documented runtime projection explicitly.
std::uint64_t loaded_bits(std::uint64_t bits) {
    if ((bits & 0x7FF0000000000000ULL) == 0x7FF0000000000000ULL &&
        (bits & 0x000FFFFFFFFFFFFFULL) != 0) {
        bits |= 0x0008000000000000ULL;
    }
    return bits;
}
void write_expected(std::array<std::uint8_t, 0x3A8>& expected,
                    const lemmings::LevelEntityPrefix::ActorRecord& actor, int mode) {
    for (int i = 0; i != 3; ++i) store(expected, 8 + 8 * i, loaded_bits(actor.position_bits[i]));
    store(expected, 0x20, loaded_bits(actor.field_20_bits));
    store(expected, 0x30, loaded_bits(actor.field_30_bits));
    store(expected, 0x38, static_cast<std::uint8_t>(actor.flag_38));
    store(expected, 0x28, static_cast<std::uint8_t>(actor.flag_28));
    store(expected, 0x3A, actor.id);
    if (mode == 0) return;
    for (int i = 0; i != 2; ++i) store(expected, 0x40 + 4 * i, actor.values_40_44[i]);
    const std::size_t flag_offsets[] = {0x370, 0x48, 0x49, 0x4A};
    for (int i = 0; i != 4; ++i) store(expected, flag_offsets[i],
        static_cast<std::uint8_t>(actor.flags_370_48_49_4a[i]));
    store(expected, 0x4C, actor.value_4c);
    store(expected, 0x5C, static_cast<std::uint8_t>(actor.flag_5c));
    store(expected, 0x5F, static_cast<std::uint8_t>(actor.flag_5f));
    const std::size_t pair_offsets[] = {0x68, 0x70, 0x350, 0x358, 0x360, 0x368};
    for (int i = 0; i != 6; ++i) store(expected, pair_offsets[i],
        loaded_bits(actor.pairs_68_350_360[i]));
    store(expected, 0x340, reinterpret_cast<std::uintptr_t>(
        fail_allocation ? nullptr : allocation.data()));
    if (mode == 2) {
        store(expected, 0x3A4, static_cast<std::uint8_t>(actor.subclass_flag_3a4));
        store(expected, 0x3A0, std::uint32_t{0xFFFFFFFF});
    }
}
} // namespace

int main() {
    _setmode(_fileno(stdin), _O_BINARY);
    void* image = VirtualAlloc(reinterpret_cast<void*>(image_base), image_size,
                               MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (image != reinterpret_cast<void*>(image_base)) return 3;
    if (!read_at<30>(0x00402540) || !read_at<30>(0x00402560) ||
        !read_at<30>(0x00402580) || !read_at<156>(0x00423291) ||
        !read_at<31>(0x00423360) || !read_at<25>(0x00423380) ||
        !read_at<189>(0x004265E8) || !read_at<391>(0x0042FB5F) ||
        !read_at<29>(0x0047DAC8) || !read_at<54>(0x00480BB0) ||
        !read_at<40>(0x004821DE)) {
        std::cerr << "expected verified function bytes on stdin\n";
        return 2;
    }
    write_jump(0x0047EEF7, reinterpret_cast<std::uintptr_t>(&read_adapter));
    write_jump(0x0049C1A8, reinterpret_cast<std::uintptr_t>(&allocation_adapter));
    write_jump(0x00415FA0, reinterpret_cast<std::uintptr_t>(&constructor_adapter));
    write_jump(0x0040BAC0, reinterpret_cast<std::uintptr_t>(&constructor_adapter));
    write_jump(0x00430200, reinterpret_cast<std::uintptr_t>(&reset_adapter));
    write_jump(0x0043B517, reinterpret_cast<std::uintptr_t>(&terrain_adapter));
    write_jump(0x0042E696, reinterpret_cast<std::uintptr_t>(&attachment_adapter));
    DWORD old_protection = 0;
    if (!VirtualProtect(image, 0xC0000, PAGE_EXECUTE_READ, &old_protection)) return 2;
    FlushInstructionCache(GetCurrentProcess(), image, 0xC0000);
    const std::uintptr_t addresses[] = {0x00423291, 0x0042FB5F, 0x004265E8};
    std::vector<void*> registry(65536, nullptr);
    auto* registry_zero = registry.data() + 32768;
    std::memcpy(reinterpret_cast<void*>(0x004CBB24), &registry_zero, sizeof(registry_zero));
    auto* counter = reinterpret_cast<std::uint16_t*>(0x004C55EC);
    std::mt19937_64 random(0x423291);
    const std::uint64_t special[] = {0, 0x8000000000000000ULL, 1,
        0x8000000000000001ULL, 0x000FFFFFFFFFFFFFULL, 0x0010000000000000ULL,
        0x7FEFFFFFFFFFFFFFULL, 0xFFEFFFFFFFFFFFFFULL, 0x7FF0000000000000ULL,
        0xFFF0000000000000ULL, 0x7FF8000000001234ULL, 0xFFF8000000004321ULL,
        0x7FF0000000000001ULL, 0xFFF0000000001234ULL};
    int comparisons = 0;
    for (int trial = 0; trial != 10000; ++trial) {
        std::vector<std::uint8_t> record;
        const auto next_double = [&](int field) {
            return trial < 14 ? special[(trial + field) % 14] : random();
        };
        for (int i = 0; i != 5; ++i) put(record, next_double(i));
        put(record, static_cast<std::uint8_t>(trial));
        put(record, static_cast<std::uint8_t>(255 - trial));
        const auto id = static_cast<std::uint16_t>(trial < 4
            ? std::array<unsigned, 4>{0, 32767, 32768, 65535}[trial] : random());
        put(record, id);
        put(record, static_cast<std::uint32_t>(random()));
        put(record, static_cast<std::uint32_t>(random()));
        for (int i = 0; i != 4; ++i) put(record, static_cast<std::uint8_t>(trial + i));
        put(record, static_cast<std::uint32_t>(random()));
        put(record, static_cast<std::uint8_t>(trial * 7));
        put(record, static_cast<std::uint8_t>(trial * 11));
        for (int i = 0; i != 6; ++i) put(record, next_double(i + 5));
        put(record, static_cast<std::uint8_t>(trial));

        for (int mode = 0; mode != 3; ++mode) {
            // Zero registry size and empty preliminary tables; a single record
            // in the relevant actor list exercises the public source API.
            std::vector<std::uint8_t> wrapped(12, 0);
            put(wrapped, std::uint32_t{mode == 2 ? 0U : 1U});
            if (mode != 2) wrapped.insert(wrapped.end(), record.begin(), record.begin() + 110);
            put(wrapped, std::uint32_t{mode == 2 ? 1U : 0U});
            if (mode == 2) wrapped.insert(wrapped.end(), record.begin(), record.end());
            lemmings::LevelByteStream source_stream{wrapped};
            const auto parsed = lemmings::read_level_entity_prefix(source_stream);
            const auto& actor = mode == 2 ? parsed.subclass_actors.front() : parsed.actors.front();
            if (source_stream.cursor != wrapped.size()) return 1;

            std::array<std::uint8_t, 0x3A8> original{}, expected{};
            original.fill(0xA5);
            expected.fill(0xA5);
            fail_allocation = trial % 2 == 0;
            allocations = constructors = resets = terrain_calls = attachments = 0;
            write_expected(expected, actor, mode);
            const auto length = mode == 0 ? 44U : mode == 1 ? 110U : 111U;
            MemoryStream stream{record.data(), length, 0};
            const auto initial_counter = static_cast<std::uint16_t>(trial < 2 ? 65534 + trial : random());
            *counter = initial_counter;
            registry_zero[actor.id] = nullptr;
            reinterpret_cast<NativeRead>(addresses[mode])(original.data(), &stream);
            ++comparisons;
            const int expected_allocations = mode == 0 ? 0 :
                1 + (mode == 2 && actor.subclass_flag_3a4 ? 1 : 0);
            if (original != expected || stream.cursor != length ||
                *counter != static_cast<std::uint16_t>(initial_counter + 1) ||
                registry_zero[actor.id] != original.data() ||
                allocations != expected_allocations ||
                constructors != (fail_allocation ? 0 : expected_allocations) ||
                resets != (mode != 0) || terrain_calls != (mode != 0) ||
                attachments != (mode == 2 && actor.subclass_flag_3a4)) {
                std::cerr << "entity mismatch trial=" << trial << " mode=" << mode
                          << " cursor=" << stream.cursor << '\n';
                for (std::size_t i = 0; i < original.size(); ++i) {
                    if (original[i] != expected[i]) {
                        std::cerr << "first offset=" << i << " native=" << unsigned(original[i])
                                  << " expected=" << unsigned(expected[i]) << '\n';
                        break;
                    }
                }
                return 1;
            }
        }
    }
    VirtualFree(image, 0, MEM_RELEASE);
    std::cout << "native entity readers passed " << comparisons
              << " comparisons (10000 each for 44/110/111-byte records)\n";
}
