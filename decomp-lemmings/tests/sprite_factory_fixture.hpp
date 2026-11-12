#pragma once

#include "lemmings/sprite_factory.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

namespace sprite_factory_test {
using namespace lemmings;

struct Terminated {};

inline void require(bool condition) {
    if (!condition) std::abort();
}

struct Fixture {
    SpriteFactoryResource resource;
    std::array<char, 256> name{};
    std::vector<std::uint64_t> trace;
    std::string formatted_path, reported_path, source_path;
    SpriteFactoryFile* file = nullptr;
    char* path_buffer = nullptr;
    unsigned trial, operation, flags;
    std::uint32_t open_result;
    bool allocation_null, mutate_name, name_alias_resource, fail_open;

    explicit Fixture(unsigned value, bool failure = false)
        : trial(value), operation(value % 7), flags((value / 7) % 6),
          open_result(1), allocation_null((value / 42) % 3 == 0),
          mutate_name((value / 126) % 2 != 0),
          name_alias_resource((value / 252) % 5 == 1), fail_open(failure) {
        const unsigned flag_values[] = {0, 1, 2, 3, 0x80000000u, 0xffffffffu};
        flags = flag_values[flags];
        const std::uint32_t successes[] = {1, 0xff, 0x10001, 0xffffff80u, 0xffffffffu};
        const std::uint32_t failures[] = {0, 0x100, 0xffffff00u};
        open_result = failure ? failures[value % 3] : successes[(value / 1260) % 5];
        if (failure) operation = 0;
        resource = {{0x00494e49u, 0x11223344, 0x55667788,
                     reinterpret_cast<void*>(std::uintptr_t(0x12345678))},
                    {0x99aabbcc, 0xddeeff00, 0x13579bdf,
                     reinterpret_cast<void*>(std::uintptr_t(0x23456789))}};
        const unsigned lengths[] = {0, 1, 19, 247};
        const auto length = lengths[(value / 6300) % 4];
        for (unsigned i = 0; i < length; ++i) name[i] = i % 13 == 0 ? '%' : char('a' + i % 26);
    }

    const char* input_name() {
        return name_alias_resource ? reinterpret_cast<const char*>(&resource) : name.data();
    }

    static std::uint64_t pointer(void* p) { return reinterpret_cast<std::uintptr_t>(p); }
    void append_array(const SpriteRuntimeArray& array) {
        trace.insert(trace.end(), {array.count_00, array.capacity_04,
                                  array.growth_08, pointer(array.data_0c)});
    }
    void append_resource() { append_array(resource.clips_00); append_array(resource.triple_arrays_10); }

    SpriteFactoryResource* allocate(std::size_t bytes) {
        require(bytes == 32);
        trace.push_back(10);
        append_resource();
        if (mutate_name) {
            if (name_alias_resource) {
                // Keep the embedded name's terminator, including for a null allocation.
                resource.clips_00.capacity_04 = 0x48474645;
            } else {
                std::strcpy(name.data(), "changed%by_allocator");
            }
        }
        return allocation_null ? nullptr : &resource;
    }
    void format(char* output, const char* pattern, const char* input) {
        require(std::strcmp(pattern, "Sprites\\%s.spr") == 0);
        require(input == input_name());
        trace.push_back(20);
        append_resource();
        std::sprintf(output, pattern, input);
        path_buffer = output;
        formatted_path = output;
        // Observe that initialization already happened, then alter the resource
        // to detect an incorrect second initialization before reading.
        resource.clips_00.count_00 ^= 0x10203040;
    }
    void construct_file(SpriteFactoryFile& object, const char* path) {
        require(path == path_buffer);
        trace.push_back(30);
        file = &object;
        object.opaque.fill(std::byte{0xcd});
        object.opaque[0] = std::byte{0x31};
    }
    std::uint32_t open_file(SpriteFactoryFile& object, std::uint32_t first,
                            std::uint32_t second) {
        require(&object == file && first == 1 && second == 0);
        require(object.opaque[0] == std::byte{0x31});
        trace.push_back(40);
        object.opaque[0] = std::byte{0x41};
        // The report must use the same live filename buffer.
        if (trial % 2) path_buffer[0] = 's';
        return open_result;
    }
    void read_resource(SpriteFactoryResource* result, SpriteFactoryFile& object) {
        require(&object == file && object.opaque[0] == std::byte{0x41});
        require(result == (allocation_null ? nullptr : &resource));
        trace.push_back(50);
        trace.push_back(result != nullptr);
        append_resource();
        resource.triple_arrays_10.count_00 = 123;
        resource.triple_arrays_10.data_0c = reinterpret_cast<void*>(std::uintptr_t(0x76543210));
        object.opaque[0] = std::byte{0x51};
    }
    void close_file(SpriteFactoryFile& object) {
        require(&object == file && object.opaque[0] == std::byte{0x51});
        trace.push_back(60);
        append_resource();
        resource.clips_00.growth_08 = 27;
        object.opaque[0] = std::byte{0x61};
    }
    void destroy_file(SpriteFactoryFile& object) {
        require(&object == file && object.opaque[0] == std::byte{0x61});
        trace.push_back(70);
        append_resource();
        resource.clips_00.capacity_04 = 0xbeef;
    }
    void report(const char* source, std::uint32_t line, const char* path) {
        require(line == 31 && path == path_buffer);
        trace.push_back(80);
        append_resource();
        source_path = source;
        reported_path = path;
    }
    void terminating(int status) {
        require(status == 0);
        trace.push_back(90);
    }
    void clear_triples(SpriteRuntimeArray& array) {
        require(&array == &resource.triple_arrays_10);
        trace.push_back(100);
        append_array(array);
        array.count_00 = 0;
        array.data_0c = nullptr;
        resource.clips_00.count_00 = 0x87654321;
    }
    void clear_clips(SpriteRuntimeArray& array) {
        require(&array == &resource.clips_00);
        trace.push_back(110);
        append_array(array);
        require(array.count_00 == 0x87654321);
        array.count_00 = 0;
        array.data_0c = nullptr;
    }
    void free_resource(SpriteFactoryResource* object) {
        require(object == &resource);
        trace.push_back(120);
        append_resource();
        // Retain test storage to inspect post-call data and the returned pointer.
        resource.clips_00.growth_08 = 0xdeadbeef;
    }

    SpriteFactoryHooks hooks() {
        return {this,
            [](void* c, std::size_t n) { return static_cast<Fixture*>(c)->allocate(n); },
            [](void* c, char* d, const char* f, const char* n) { static_cast<Fixture*>(c)->format(d,f,n); },
            [](void* c, SpriteFactoryFile& f, const char* p) { static_cast<Fixture*>(c)->construct_file(f,p); },
            [](void* c, SpriteFactoryFile& f, std::uint32_t a, std::uint32_t b) { return static_cast<Fixture*>(c)->open_file(f,a,b); },
            [](void* c, SpriteFactoryResource* r, SpriteFactoryFile& f) { static_cast<Fixture*>(c)->read_resource(r,f); },
            [](void* c, SpriteFactoryFile& f) { static_cast<Fixture*>(c)->close_file(f); },
            [](void* c, SpriteFactoryFile& f) { static_cast<Fixture*>(c)->destroy_file(f); },
            [](void* c, const char* s, std::uint32_t l, const char* p) { static_cast<Fixture*>(c)->report(s,l,p); },
            [](void* c, int s) { static_cast<Fixture*>(c)->terminating(s); throw Terminated{}; }};
    }
    SpriteFactoryDestroyHooks destroy_hooks() {
        return {this,
            [](void* c, SpriteRuntimeArray& a) { static_cast<Fixture*>(c)->clear_clips(a); },
            [](void* c, SpriteRuntimeArray& a) { static_cast<Fixture*>(c)->clear_triples(a); },
            [](void* c, SpriteFactoryResource* r) { static_cast<Fixture*>(c)->free_resource(r); }};
    }
    unsigned run() {
        switch (operation) {
        case 0: return create_sprite_resource(input_name(), hooks()) != nullptr;
        case 1: return initialize_sprite_resource(resource) == &resource;
        case 2: return initialize_sprite_array(resource.clips_00) == &resource.clips_00;
        case 3: return initialize_sprite_array(resource.triple_arrays_10) == &resource.triple_arrays_10;
        case 4: destroy_sprite_resource_contents(resource, destroy_hooks()); return 0;
        case 5: return delete_sprite_resource(&resource, flags, destroy_hooks()) == &resource;
        default: destroy_sprite_resource(allocation_null ? nullptr : &resource, destroy_hooks()); return 0;
        }
    }
    std::vector<std::uint64_t> snapshot() const {
        const auto& a = resource.clips_00;
        const auto& b = resource.triple_arrays_10;
        return {a.count_00,a.capacity_04,a.growth_08,pointer(a.data_0c),
                b.count_00,b.capacity_04,b.growth_08,pointer(b.data_0c)};
    }
    bool equivalent(const Fixture& other) const {
        return trace == other.trace && snapshot() == other.snapshot() &&
               formatted_path == other.formatted_path && reported_path == other.reported_path &&
               source_path == other.source_path && name == other.name;
    }
};
} // namespace sprite_factory_test
