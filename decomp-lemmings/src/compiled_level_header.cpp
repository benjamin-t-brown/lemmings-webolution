#include "lemmings/compiled_level_header.hpp"

#include <cstring>
#include <stdexcept>

namespace lemmings {
namespace {
std::uint32_t read_u32(LevelByteStream& stream) {
    if (stream.cursor > stream.bytes.size() ||
        stream.bytes.size() - stream.cursor < 4) {
        throw std::invalid_argument("truncated level header");
    }
    const auto* bytes = stream.bytes.data() + stream.cursor;
    stream.cursor += 4;
    return static_cast<std::uint32_t>(bytes[0]) |
           (static_cast<std::uint32_t>(bytes[1]) << 8) |
           (static_cast<std::uint32_t>(bytes[2]) << 16) |
           (static_cast<std::uint32_t>(bytes[3]) << 24);
}
}

CompiledLevelHeader read_compiled_level_header(LevelByteStream& stream) {
    CompiledLevelHeader result{};
    char marker[255]{};
    read_level_string(stream, marker, 255);
    result.marker_matches = std::strcmp(marker, "Level 23") == 0 && marker[0] != 0;
    for (auto& field : result.fields) field = read_u32(stream);
    result.first_field_times_58 = result.fields[0] * 58u;
    for (auto& flag : result.flags) {
        if (stream.cursor >= stream.bytes.size()) {
            throw std::invalid_argument("truncated level flags");
        }
        flag = stream.bytes[stream.cursor++] != 0;
    }
    return result;
}

} // namespace lemmings
