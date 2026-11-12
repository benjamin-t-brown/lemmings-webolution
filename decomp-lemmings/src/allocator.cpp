#include "lemmings/allocator.hpp"

#include <new>

namespace lemmings {
namespace {

void* host_allocate(std::size_t bytes) {
    return ::operator new(bytes);
}

void host_release(void* block) {
    ::operator delete(block);
}

Allocator g_active{&host_allocate, &host_release};

} // namespace

const Allocator& active_allocator() {
    return g_active;
}

Allocator set_active_allocator(Allocator allocator) {
    const Allocator previous = g_active;
    if (allocator.allocate != nullptr && allocator.release != nullptr) {
        g_active = allocator;
    }
    return previous;
}

Allocator host_allocator() {
    return Allocator{&host_allocate, &host_release};
}

} // namespace lemmings
