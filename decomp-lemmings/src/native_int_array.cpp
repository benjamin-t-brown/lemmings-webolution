#include "lemmings/native_int_array.hpp"

#include "lemmings/allocator.hpp"

#include <cstring>
#include <limits>
#include <new>
#include <stdexcept>

namespace lemmings {

void clear_native_int_array(NativeIntArray& array) {
    if (array.data != nullptr) {
        array.capacity = 0;
        active_allocator().release(array.data);
        array.data = nullptr;
    }
    array.count = 0;
}

void append_native_int(NativeIntArray& array, std::int32_t value) {
    if (array.count >= array.capacity) {
        // The original does not check this state before allocating. Reject
        // invalid/corrupt headers here instead of writing out of bounds.
        const auto new_capacity = static_cast<std::int64_t>(array.capacity) + array.growth;
        if (new_capacity <= array.count ||
            new_capacity > std::numeric_limits<std::int32_t>::max()) {
            throw std::length_error("invalid native array growth");
        }
        const Allocator& allocator = active_allocator();
        auto* replacement = static_cast<std::int32_t*>(allocator.allocate(
            static_cast<std::size_t>(new_capacity) * sizeof(std::int32_t)));
        if (array.count > 0) {
            std::memcpy(replacement, array.data,
                        static_cast<std::size_t>(array.count) * sizeof(std::int32_t));
        }
        allocator.release(array.data);
        array.data = replacement;
        array.capacity = static_cast<std::int32_t>(new_capacity);
    }
    array.data[array.count] = value;
    ++array.count;
}

} // namespace lemmings
