#include "lemmings/level_stream.hpp"

#include <algorithm>
#include <cstring>
#include <stdexcept>

namespace lemmings {

void read_level_string(LevelByteStream& stream, char* destination,
                       std::int32_t capacity) {
    if (capacity < 1 || destination == nullptr ||
        stream.cursor > stream.bytes.size() ||
        stream.bytes.size() - stream.cursor < 4) {
        throw std::invalid_argument("invalid level string read");
    }
    std::int32_t length = 0;
    std::memcpy(&length, stream.bytes.data() + stream.cursor, 4);
    stream.cursor += 4;
    if (capacity < length) {
        destination[0] = 0;
        return;
    }
    const auto copy_length = std::max<std::int32_t>(0,
        std::min(length, capacity - 1));
    if (stream.bytes.size() - stream.cursor < static_cast<std::size_t>(std::max(length, 0))) {
        throw std::invalid_argument("truncated level string");
    }
    for (std::int32_t i = 0; i < copy_length; ++i) {
        destination[i] = static_cast<char>(stream.bytes[stream.cursor++]);
    }
    destination[copy_length] = 0;
    stream.cursor += static_cast<std::size_t>(std::max(length, 0) - copy_length);
}

} // namespace lemmings
