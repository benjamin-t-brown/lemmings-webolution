#pragma once

#include "lemmings/sprite_resource_destruction.hpp"
#include <cstring>
#include <cstdlib>
#include <memory>
#include <vector>

namespace sprite_destruction_test {
using namespace lemmings;
inline void require(bool condition) { if (!condition) std::abort(); }

enum class Kind { clips, frames, pieces, triples, data };
struct Allocation {
    std::unique_ptr<std::byte[]> storage;
    std::byte* cookie;
    void* elements;
    std::size_t stride;
    Kind kind;
};

struct Fixture {
    static constexpr unsigned physical_count = 3;
    SpriteFactoryResource resource;
    SpriteDestructionClip* clips;
    SpriteDestructionTripleArray* triples;
    std::array<SpriteDestructionFrame*, 3> frames;
    std::array<std::array<SpriteDestructionPiece*, 3>, 3> pieces;
    std::vector<Allocation> allocations;
    std::vector<std::uint64_t> trace;
    std::vector<unsigned> freed;
    SpriteRuntimeArray* target_header;
    unsigned trial, operation, flags, cookie_count, null_mode, mutation;

    template<class T> T* allocate(Kind kind) {
        auto storage = std::make_unique<std::byte[]>(sizeof(T) * physical_count + alignof(T) + 4);
        const auto start = reinterpret_cast<std::uintptr_t>(storage.get()) + 4;
        const auto aligned = (start + alignof(T) - 1) & ~(std::uintptr_t(alignof(T)) - 1);
        auto* elements = reinterpret_cast<T*>(aligned);
        auto* cookie = reinterpret_cast<std::byte*>(elements) - 4;
        std::memcpy(cookie, &cookie_count, 4);
        for (unsigned i = 0; i < physical_count; ++i) std::construct_at(elements + i);
        allocations.push_back({std::move(storage), cookie, elements, sizeof(T), kind});
        return elements;
    }
    SpriteRuntimeArray header(void* data, unsigned salt) const {
        const std::uint32_t counts[] = {0, 1, 9, 0xffffffffu, 0x80000000u};
        return {counts[(trial / 312) % 5], 0x9000u + salt, 0x1200u + salt,
                null_mode == 1 || (null_mode == 2 && salt % 2) ? nullptr : data};
    }
    explicit Fixture(unsigned t) : trial(t), operation(t % 13), flags((t / 52) % 6),
        cookie_count((t / 13) % 4), null_mode((t / 1560) % 3), mutation((t / 4680) % 4) {
        const unsigned values[] = {0,1,2,3,0x80000000u,0xffffffffu};
        flags = values[flags];
        clips = allocate<SpriteDestructionClip>(Kind::clips);
        triples = allocate<SpriteDestructionTripleArray>(Kind::triples);
        for (unsigned c = 0; c < 3; ++c) {
            frames[c] = allocate<SpriteDestructionFrame>(Kind::frames);
            clips[c].frames_00 = header(frames[c], 10 + c);
            clips[c].speed_bits_10.fill(std::byte(0x40 + c));
            clips[c].name_18.fill(char('a' + c));
            for (unsigned f = 0; f < 3; ++f) {
                pieces[c][f] = allocate<SpriteDestructionPiece>(Kind::pieces);
                frames[c][f].pieces_00 = header(pieces[c][f], 20 + c * 3 + f);
                for (unsigned p = 0; p < 3; ++p)
                    pieces[c][f][p].opaque_00.fill(std::byte(1 + c * 9 + f * 3 + p));
            }
            auto* data = allocate<std::array<std::byte, 24>>(Kind::data);
            triples[c] = {0x80000000u + c, null_mode == 1 || (null_mode == 2 && c % 2) ? nullptr : data};
        }
        resource = {header(clips, 0), header(triples, 1)};
        target_header = operation == 1 || operation == 5 || operation == 9
            ? &resource.triple_arrays_10
            : operation == 2 || operation == 6 || operation == 8 ? &clips[0].frames_00
            : operation == 3 || operation == 7 || operation == 10 ? &frames[0][0].pieces_00
            : &resource.clips_00;
    }

    unsigned pointer(const void* p) const {
        if (p == nullptr) return 0;
        for (unsigned n = 0; n < allocations.size(); ++n) {
            const auto& a = allocations[n];
            if (p == a.cookie) return (n + 1) * 100;
            for (unsigned i = 0; i < physical_count; ++i)
                if (p == static_cast<std::byte*>(a.elements) + i * a.stride) return (n + 1) * 100 + i + 1;
        }
        std::abort();
    }
    void append_header(std::vector<std::uint64_t>& output, const SpriteRuntimeArray& h) const {
        output.insert(output.end(), {h.count_00,h.capacity_04,h.growth_08,pointer(h.data_0c)});
    }
    std::vector<std::uint64_t> snapshot() const {
        std::vector<std::uint64_t> result;
        append_header(result, resource.clips_00);
        append_header(result, resource.triple_arrays_10);
        for (const auto& a : allocations) {
            std::uint32_t cookie;
            std::memcpy(&cookie, a.cookie, 4);
            result.push_back(cookie);
            for (unsigned i = 0; i < 3; ++i) {
                if (a.kind == Kind::clips) {
                    const auto& clip = static_cast<SpriteDestructionClip*>(a.elements)[i];
                    append_header(result, clip.frames_00);
                    for (auto b : clip.speed_bits_10) result.push_back(std::to_integer<unsigned>(b));
                    for (auto ch : clip.name_18) result.push_back(static_cast<unsigned char>(ch));
                } else if (a.kind == Kind::frames) {
                    append_header(result, static_cast<SpriteDestructionFrame*>(a.elements)[i].pieces_00);
                } else if (a.kind == Kind::triples) {
                    const auto& triple = static_cast<SpriteDestructionTripleArray*>(a.elements)[i];
                    result.insert(result.end(), {triple.type_00,pointer(triple.data_04)});
                } else {
                    auto* bytes = static_cast<std::byte*>(a.elements) + i * a.stride;
                    for (std::size_t j = 0; j < a.stride; ++j) result.push_back(std::to_integer<unsigned>(bytes[j]));
                }
            }
        }
        return result;
    }
    void free_memory(void* p) {
        const auto id = pointer(p);
        require(id != 0);
        freed.push_back(id);
        trace.push_back(id);
        append_header(trace, *target_header);
        append_header(trace, clips[0].frames_00);
        trace.push_back(pointer(triples[0].data_04));
        if (mutation == 1) {
            target_header->count_00 = 0xbadc0de;
            target_header->capacity_04 = 0x77110000u + id;
            target_header->growth_08 ^= id;
            target_header->data_0c = nullptr;
        } else if (mutation == 2 && freed.size() == 1) {
            // Vector-delete captures its cookie before entering element cleanup.
            std::uint32_t zero = 0;
            for (auto& allocation : allocations) std::memcpy(allocation.cookie, &zero, 4);
        } else if (mutation == 3 && freed.size() == 1) {
            // Remaining elements are loaded at the time their destructor runs.
            clips[0].frames_00.data_0c = nullptr;
            triples[0].data_04 = nullptr;
        }
        // Test allocations stay alive to compare post-destruction snapshots.
    }
    SpriteResourceDestructionHooks hooks() {
        return {this, [](void* c, void* p) { static_cast<Fixture*>(c)->free_memory(p); }};
    }
    SpriteFactoryDestroyHooks factory_hooks() {
        return {this,
            [](void* c, SpriteRuntimeArray& h) { auto& f=*static_cast<Fixture*>(c); clear_sprite_clips(h,f.hooks()); },
            [](void* c, SpriteRuntimeArray& h) { auto& f=*static_cast<Fixture*>(c); clear_sprite_triple_arrays(h,f.hooks()); },
            [](void*, SpriteFactoryResource*) { std::abort(); }};
    }
    unsigned run() {
        switch (operation) {
        case 0: clear_sprite_clips(resource.clips_00,hooks()); break;
        case 1: clear_sprite_triple_arrays(resource.triple_arrays_10,hooks()); break;
        case 2: clear_sprite_frames(clips[0].frames_00,hooks()); break;
        case 3: clear_sprite_pieces(frames[0][0].pieces_00,hooks()); break;
        case 4: return pointer(delete_sprite_clips(clips,flags,hooks()));
        case 5: return pointer(delete_sprite_triple_arrays(triples,flags,hooks()));
        case 6: return pointer(delete_sprite_frames(frames[0],flags,hooks()));
        case 7: return pointer(delete_sprite_pieces(pieces[0][0],flags,hooks()));
        case 8: destroy_sprite_clip(clips[0],hooks()); break;
        case 9: destroy_sprite_triple_array(triples[0],hooks()); break;
        case 10: destroy_sprite_frame(frames[0][0],hooks()); break;
        case 11: destroy_sprite_piece(pieces[0][0][0]); break;
        case 12: destroy_sprite_resource_contents(resource,factory_hooks()); break;
        }
        return 0;
    }
};
} // namespace sprite_destruction_test
