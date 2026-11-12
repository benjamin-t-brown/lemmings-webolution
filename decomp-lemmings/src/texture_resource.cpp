#include "lemmings/texture_resource.hpp"

#include <array>
#include <stdexcept>

namespace lemmings {
namespace {
[[noreturn]] void invalid() { throw std::invalid_argument("invalid packed texture data"); }

struct Bits {
    std::span<const std::uint8_t> bytes;
    std::size_t cursor = 0;
    unsigned bit = 0;
    unsigned next() {
        if (cursor >= bytes.size()) invalid();
        const auto value = (bytes[cursor] >> (7 - bit)) & 1u;
        if (++bit == 8) { bit = 0; ++cursor; }
        return value;
    }
    std::uint32_t take(unsigned count) {
        std::uint32_t value = 0;
        while (count--) value = (value << 1) | next();
        return value;
    }
};

struct Table {
    std::array<std::uint8_t, 17> lengths{};
    std::array<std::uint16_t, 17> codes{};
    unsigned symbols = 0;
    unsigned maximum = 0;

    void read(std::span<const std::uint8_t> bytes, std::size_t& cursor) {
        if (cursor >= bytes.size()) invalid();
        const unsigned first = bytes[cursor++];
        if (first == 0) return;
        if ((first & 15u) == 0) invalid();
        symbols = (first & 15u) + 2;
        lengths[0] = first >> 4;
        for (unsigned i = 1; i < symbols; ++i) {
            if (cursor >= bytes.size()) invalid();
            if (i & 1u) lengths[i] = bytes[cursor] & 15u;
            else lengths[i] = bytes[cursor++] >> 4;
        }
        if ((symbols & 1u) == 0) ++cursor;
        std::array<unsigned, 16> counts{}, next{};
        for (unsigned i = 0; i < symbols; ++i) {
            if (lengths[i] != 0) ++counts[lengths[i]];
            if (lengths[i] > maximum) maximum = lengths[i];
        }
        unsigned code = 0;
        for (unsigned length = 1; length <= maximum; ++length) {
            code = (code + counts[length - 1]) << 1;
            if (code + counts[length] > (1u << length)) invalid();
            next[length] = code;
        }
        if (maximum == 0) invalid();
        for (unsigned i = 0; i < symbols; ++i)
            if (lengths[i]) codes[i] = static_cast<std::uint16_t>(next[lengths[i]]++);
    }
    std::uint32_t value(Bits& input) const {
        if (symbols == 0) {
            unsigned count = 0;
            while (input.next()) if (++count > 15) invalid();
            if (count == 0) return 0;
            return ((1u << count) | input.take(count)) - 1;
        }
        unsigned code = 0;
        for (unsigned length = 1; length <= maximum; ++length) {
            code = (code << 1) | input.next();
            for (unsigned symbol = 0; symbol < symbols; ++symbol) {
                if (lengths[symbol] == length && codes[symbol] == code) {
                    if (symbol < 2) return symbol;
                    return (1u << (symbol - 1)) | input.take(symbol - 1);
                }
            }
        }
        invalid();
    }
};

std::uint16_t u16(std::span<const std::uint8_t> bytes, std::size_t at) {
    if (at > bytes.size() || bytes.size() - at < 2) invalid();
    return static_cast<std::uint16_t>(bytes[at] | (unsigned(bytes[at + 1]) << 8));
}
} // namespace

std::vector<std::uint8_t> decode_texture_block(std::span<const std::uint8_t> bytes,
                                             std::size_t expected_size) {
    if (expected_size == 0 || expected_size > 0xffffffffu) invalid();
    std::array<Table, 3> tables;
    std::size_t cursor = 0;
    for (auto& table : tables) table.read(bytes, cursor);
    Bits input{bytes, cursor, 0};
    std::vector<std::uint8_t> result;
    result.reserve(expected_size);
    while (result.size() < expected_size) {
        const auto literals = tables[0].value(input);
        if (literals > expected_size - result.size()) invalid();
        for (std::uint32_t i = 0; i < literals; ++i)
            result.push_back(static_cast<std::uint8_t>(input.take(8)));
        if (result.size() == expected_size) break;
        const auto length = tables[1].value(input) + 2;
        const auto distance = tables[2].value(input) + 1;
        if (length > expected_size - result.size() || distance > result.size()) invalid();
        for (std::uint32_t i = 0; i < length; ++i)
            result.push_back(result[result.size() - distance]);
    }
    return result;
}

std::vector<std::uint8_t> decode_texture_bitmap(std::span<const std::uint8_t> bytes) {
    if (bytes.size() >= 2 && bytes[0] == 'B' && bytes[1] == 'M')
        return {bytes.begin(), bytes.end()};
    if (bytes.empty()) invalid();
    std::size_t cursor = 1, output_size = 0;
    for (unsigned i = 0; i < bytes[0]; ++i) {
        const auto size = u16(bytes, cursor);
        output_size += u16(bytes, cursor + 2);
        if (size < 5 || size > bytes.size() - cursor) invalid();
        cursor += size;
    }
    std::vector<std::uint8_t> result;
    result.reserve(output_size);
    cursor = 1;
    for (unsigned i = 0; i < bytes[0]; ++i) {
        const auto size = u16(bytes, cursor);
        const auto decoded_size = u16(bytes, cursor + 2);
        const auto payload = bytes.subspan(cursor + 5, size - 5);
        if (bytes[cursor + 4] == 3) {
            if (payload.size() != decoded_size) invalid();
            result.insert(result.end(), payload.begin(), payload.end());
        } else if (bytes[cursor + 4] == 4) {
            const auto block = decode_texture_block(payload, decoded_size);
            result.insert(result.end(), block.begin(), block.end());
        } else invalid();
        cursor += size;
    }
    return result;
}
} // namespace lemmings
