#pragma once
#include "lemmings/texture_creation.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <source_location>
#include <string>
#include <vector>

namespace texture_creation_test {
using namespace lemmings;
inline void require(bool value, std::source_location site = std::source_location::current()) {
    if (!value) { std::cerr << site.file_name() << ':' << site.line() << " failed\n"; std::exit(1); }
}
struct Terminated {};
struct Fixture {
    TextureCreationResource resource;
    std::array<ResourceCacheEntry, 3> entries{};
    ResourceCache cache{entries.data()};
    std::array<std::byte, 4096> bmp{}, scratch{}, output{}, alpha{}, alternate{};
    TextureBitmapBuffer* live_buffer = nullptr;
    std::array<TextureCreationFile*, 2> files{};
    std::vector<std::uint64_t> trace;
    std::vector<std::string> paths;
    std::uint8_t quality;
    unsigned trial, operation, mutation_event, event_number = 0, file_count = 0;
    std::uint32_t flags, probe_result, alpha_result;
    bool null_resource, fatal;
    static constexpr const char* names[] = {"", "x", "_x", "$x", "~x", "^x", "+x",
        "dir\\_INN_x", "dir\\$OUT_x", "dir\\~OUT_x", "dir\\^INN_x", "dir\\+OUT_x",
        "^dir\\x", "dir/^x", "dir\\", "inn_x", "fooOUT_bar", "fooINN_OUT_x"};
    explicit Fixture(unsigned value, bool failure = false)
        : quality(std::uint8_t((value / 90) % 3 == 2 ? 255 : (value / 90) % 3)),
          trial(value), operation(value % 5), mutation_event((value / 270) % 18),
          flags((value / 5) % 32), probe_result((value / 15) % 4 == 0 ? 0x100 : 0xffffff01u),
          alpha_result(failure ? (value % 3) * 256 : (value % 2 ? 0xffffff80u : 1)),
          null_resource((value / 4860) % 3 == 1), fatal(failure) {
        resource = {reinterpret_cast<void*>(0x1111), reinterpret_cast<void*>(0x2222),
            reinterpret_cast<void*>(0x3333), reinterpret_cast<void*>(0x4444), 0x55,
            {0x66, 0x77, 0x88}, 3, 5, 0x12345678, reinterpret_cast<void*>(0x9999), 0xaabbccdd};
        std::strcpy(entries[1].name_00.data(), names[(value / 5) % 18]);
        if (failure) { operation = 1; flags = 0x18; }
        for (unsigned i = 0; i < bmp.size(); ++i) bmp[i] = std::byte((i * 71 + value) & 255);
        put<std::uint32_t>(10, 64); put<std::uint32_t>(14, 40);
        put<std::int32_t>(18, 1 + (value / 18) % 7);
        put<std::int32_t>(22, 1 + (value / 126) % 5);
        put<std::uint16_t>(28, std::uint16_t(8 * (1 + value % 4)));
    }
    template<class T> void put(unsigned offset, T value) { std::memcpy(bmp.data() + offset, &value, sizeof value); }
    char* name() { return entries[1].name_00.data(); }
    std::uint64_t id(const void* p) const {
        if (!p) return 0;
        if (p == &resource) return 1;
        if (p == bmp.data()) return 2;
        if (p == scratch.data()) return 3;
        if (p == output.data()) return 4;
        if (p == alpha.data()) return 5;
        if (p == alternate.data()) return 6;
        return reinterpret_cast<std::uintptr_t>(p);
    }
    void snapshot() {
        trace.insert(trace.end(), {id(resource.device_texture_00), id(resource.bitmap_04),
            id(resource.unknown_08), id(resource.alpha_0c), resource.unknown_10,
            resource.unknown_11[0], resource.unknown_11[1], resource.unknown_11[2],
            std::uint32_t(resource.width_14), std::uint32_t(resource.height_18),
            resource.triple_type_1c, id(resource.triple_data_20), resource.flags_24, quality});
    }
    void event(unsigned code) {
        trace.push_back(code); snapshot();
        if (++event_number == mutation_event) {
            std::strcpy(name(), "+changedOUT_name");
            quality ^= 0xff;
            resource.flags_24 ^= 0xffffffffu;
            resource.width_14 = 2; resource.height_18 = 3;
            resource.alpha_0c = alternate.data();
            if (live_buffer) live_buffer->data_04 = alternate.data();
            // Pixel bytes change after the loader's shared scratch copy. The
            // source pointer and byte count must still be the captured ones.
            scratch[64] ^= std::byte{0x5a};
            const std::uint32_t changed_offset = 96;
            std::memcpy(scratch.data() + 10, &changed_offset, 4);
        }
    }
    TextureCreationResource* allocate_resource(std::uint32_t size) {
        trace.push_back(size); event(10); return null_resource ? nullptr : &resource;
    }
    void* allocate_alpha(std::uint32_t size) {
        trace.push_back(size); event(11); return trial % 13 == 0 ? nullptr : alpha.data();
    }
    void construct_file(TextureCreationFile& file, const char* path) {
        require(file_count < 2); files[file_count++] = &file;
        paths.emplace_back(path); event(20 + file_count);
    }
    unsigned file_id(TextureCreationFile& file) const {
        if (&file == files[0]) return 0;
        require(&file == files[1]); return 1;
    }
    std::uint32_t open(TextureCreationFile& file, std::uint32_t a, std::uint32_t b) {
        require(a == 1 && b == 0); const auto index = file_id(file); event(30 + index);
        return index ? alpha_result : probe_result;
    }
    void read(TextureCreationFile& file, void* data, std::uint32_t size) {
        require(file_id(file) == 1); trace.insert(trace.end(), {id(data), size}); event(40);
        // No write is required by the wrapper contract: the underlying original
        // read may return without filling the whole requested range.
    }
    void close(TextureCreationFile& file) { event(50 + file_id(file)); }
    void destroy(TextureCreationFile& file) { event(60 + file_id(file)); }
    void report(const char* source, std::uint32_t line, const char* path) {
        require(std::strcmp(source, "D:\\lrpc\\Source\\3DEngine\\TexturePC.cpp") == 0);
        require(line == 90); paths.emplace_back(path); event(70);
    }
    void terminate(int code) { require(code == 0); event(71); }
    void initialize(TextureCreationResource& a, TextureCreationResource& b) {
        require(&a == &resource && &b == &resource); event(80);
        resource.device_texture_00 = reinterpret_cast<void*>(0xa0a0);
    }
    void load(TextureBitmapBuffer& buffer, const char* path) {
        require(buffer.data_04 == nullptr); paths.emplace_back(path); event(90);
        buffer = {256, bmp.data()}; live_buffer = &buffer;
    }
    void free_data(void* p) { trace.push_back(id(p)); event(91); live_buffer = nullptr; }
    void* create_dc(void* source) { require(source == nullptr); event(100); return reinterpret_cast<void*>(0xdcdc); }
    void* create_dib(void* dc, const void* info, std::uint32_t usage, void** pixels,
                     void* section, std::uint32_t offset) {
        require(dc == reinterpret_cast<void*>(0xdcdc) && usage == 0 && section == nullptr && offset == 0);
        const auto* bytes = static_cast<const std::byte*>(info);
        for (unsigned i = 0; i < 40; ++i) trace.push_back(std::to_integer<unsigned>(bytes[i]));
        event(101); *pixels = output.data(); return reinterpret_cast<void*>(0xb1b1);
    }
    void delete_dc(void* dc) { require(dc == reinterpret_cast<void*>(0xdcdc)); event(102); }
    TextureCreationHooks hooks() {
        return {this,
            [](void* c, std::uint32_t n) { return static_cast<Fixture*>(c)->allocate_resource(n); },
            [](void* c, std::uint32_t n) { return static_cast<Fixture*>(c)->allocate_alpha(n); },
            [](void* c, TextureCreationFile& f, const char* p) { static_cast<Fixture*>(c)->construct_file(f,p); },
            [](void* c, TextureCreationFile& f, std::uint32_t a, std::uint32_t b) { return static_cast<Fixture*>(c)->open(f,a,b); },
            [](void* c, TextureCreationFile& f, void* p, std::uint32_t n) { static_cast<Fixture*>(c)->read(f,p,n); },
            [](void* c, TextureCreationFile& f) { static_cast<Fixture*>(c)->close(f); },
            [](void* c, TextureCreationFile& f) { static_cast<Fixture*>(c)->destroy(f); },
            [](void* c, const char* s, std::uint32_t n, const char* p) { static_cast<Fixture*>(c)->report(s,n,p); },
            [](void* c, int s) { static_cast<Fixture*>(c)->terminate(s); throw Terminated{}; },
            [](void* c, TextureCreationResource& a, TextureCreationResource& b) { static_cast<Fixture*>(c)->initialize(a,b); },
            {this,
            [](void* c, TextureBitmapBuffer& b, const char* p) { static_cast<Fixture*>(c)->load(b,p); },
            [](void* c, void* p) { static_cast<Fixture*>(c)->free_data(p); },
            [](void* c, void* p) { return static_cast<Fixture*>(c)->create_dc(p); },
            [](void* c, void* d, const void* i, std::uint32_t u, void** p, void* s, std::uint32_t o) { return static_cast<Fixture*>(c)->create_dib(d,i,u,p,s,o); },
            [](void* c, void* p) { static_cast<Fixture*>(c)->delete_dc(p); }, scratch.data()}, &quality};
    }
    unsigned run() {
        const auto h = hooks();
        switch (operation) {
        case 0: return create_texture_resource(name(), h) != nullptr;
        case 1: return construct_texture_resource(resource, name(), flags, h) == &resource;
        case 2: {
            auto& height = trial % 2 ? resource.width_14 : resource.height_18;
            load_texture_bitmap(resource, name(), resource.width_14, height, h.bitmap); return 0;
        }
        case 3: return texture_has_quality_variants(name());
        default: return create_texture_resource_by_index(cache, 1, h) != nullptr;
        }
    }
    bool equivalent(Fixture& other) {
        snapshot(); other.snapshot();
        return trace == other.trace && paths == other.paths && output == other.output &&
            scratch == other.scratch && std::strcmp(name(), other.name()) == 0;
    }
};
} // namespace texture_creation_test
