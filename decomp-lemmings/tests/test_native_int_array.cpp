#include "lemmings/native_int_array.hpp"

#include <stdexcept>

namespace {
void check(bool condition) {
    if (!condition) {
        throw std::runtime_error("native integer array mismatch");
    }
}
}

int main() {
    lemmings::NativeIntArray array{};
    array.capacity = 7;
    array.growth = 2;
    lemmings::clear_native_int_array(array);
    check(array.count == 0 && array.capacity == 7 && array.data == nullptr);

    array.capacity = 0;
    lemmings::append_native_int(array, 10);
    check(array.count == 1 && array.capacity == 2 && array.data[0] == 10);
    lemmings::append_native_int(array, 20);
    check(array.count == 2 && array.capacity == 2 && array.data[1] == 20);
    lemmings::append_native_int(array, 30);
    check(array.count == 3 && array.capacity == 4);
    check(array.data[0] == 10 && array.data[1] == 20 && array.data[2] == 30);

    lemmings::clear_native_int_array(array);
    check(array.count == 0 && array.capacity == 0 && array.data == nullptr);
}
