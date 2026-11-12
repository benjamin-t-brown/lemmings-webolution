#include "lemmings/sprite_runtime_read.hpp"
#include <bit>
#include <cstring>
#include <new>
#include <stdexcept>

namespace lemmings {
namespace {
std::uint64_t integer(LevelByteStream& stream,unsigned count) {
    if(stream.cursor>stream.bytes.size()||count>stream.bytes.size()-stream.cursor)
        throw std::invalid_argument("truncated runtime sprite resource");
    std::uint64_t result=0;
    for(unsigned i=0;i<count;++i)result|=std::uint64_t(stream.bytes[stream.cursor++])<<(8*i);
    return result;
}
std::uint64_t double_bits(LevelByteStream& stream) {
    auto bits=integer(stream,8);
    if((bits&0x7ff0000000000000ULL)==0x7ff0000000000000ULL && (bits&0xfffffffffffffULL))bits|=0x8000000000000ULL;
    return bits;
}
std::uint32_t count(LevelByteStream& stream) {
    auto result=static_cast<std::uint32_t>(integer(stream,4));
    if(result>0x7fffffffu)throw std::invalid_argument("negative runtime sprite array count");
    return result;
}
struct PieceTextureBridge {
    RuntimeSpritePiece& piece;
    SpriteDestructionPiece& storage;
    const ResourceRegistrationHooks& hooks;
    void publish() {std::memcpy(storage.opaque_00.data(),&piece,sizeof(piece));}
    void reload() {std::memcpy(&piece,storage.opaque_00.data(),sizeof(piece));}
};
template<class T,class Construct>
void resize_cleared(SpriteRuntimeArray& array,std::uint32_t requested,
                    SpriteReadAllocationKind kind,std::uint32_t stride,
                    const SpriteRuntimeReadHooks& hooks,Construct construct) {
    if(std::bit_cast<std::int32_t>(array.capacity_04)<static_cast<std::int32_t>(requested)) {
        if(array.data_0c || array.count_00)throw std::invalid_argument("sprite resize requires cleared array");
        if(array.growth_08==1)array.capacity_04=requested;
        else {
            if(array.growth_08==0 || array.growth_08>0x7fffffffu)throw std::invalid_argument("nonterminating sprite growth");
            do {
                if(array.capacity_04>0x7fffffffu-array.growth_08)throw std::invalid_argument("sprite growth overflow");
                array.capacity_04+=array.growth_08;
            }while(array.capacity_04<=requested);
        }
        const auto capacity=array.capacity_04;
        if(capacity>(0xffffffffu-4)/stride)throw std::invalid_argument("sprite allocation overflow");
        auto* elements=static_cast<T*>(hooks.allocate(hooks.context,kind,capacity,capacity*stride+4));
        if(!elements)throw std::bad_alloc();
        std::memcpy(reinterpret_cast<std::byte*>(elements)-4,&capacity,4);
        for(std::uint32_t i=0;i<capacity;++i)construct(elements[i]);
        array.data_0c=elements;
    }
    array.count_00=requested;
    if(requested && !array.data_0c)throw std::invalid_argument("sprite capacity without storage");
}
template<class T,class Construct>
void read_count(SpriteRuntimeArray& array,LevelByteStream& stream,SpriteReadAllocationKind kind,
                std::uint32_t stride,const SpriteRuntimeReadHooks& hooks,Construct construct) {
    const auto requested=count(stream);
    resize_cleared<T>(array,requested,kind,stride,hooks,construct);
    resize_cleared<T>(array,requested,kind,stride,hooks,construct);
}
}
void read_runtime_sprite_resource(SpriteFactoryResource& resource,LevelByteStream& stream,
                                  ResourceRegistration& textures,const ResourceRegistrationHooks& texture_hooks,
                                  const SpriteRuntimeReadHooks& hooks) {
    const SpriteResourceDestructionHooks destruction{hooks.context,hooks.free_memory};
    clear_sprite_clips(resource.clips_00,destruction);
    read_count<SpriteDestructionClip>(resource.clips_00,stream,SpriteReadAllocationKind::clips,0x98,hooks,
        [](auto& clip){initialize_sprite_array(clip.frames_00);clip.name_18[0]=0;});
    for(std::uint32_t i=0;i<resource.clips_00.count_00;++i) {
        auto& clip=static_cast<SpriteDestructionClip*>(resource.clips_00.data_0c)[i];
        clear_sprite_frames(clip.frames_00,destruction);
        read_count<SpriteDestructionFrame>(clip.frames_00,stream,SpriteReadAllocationKind::frames,0x10,hooks,
            [](auto& frame){initialize_sprite_array(frame.pieces_00);});
        for(std::uint32_t j=0;j<clip.frames_00.count_00;++j) {
            auto& frame=static_cast<SpriteDestructionFrame*>(clip.frames_00.data_0c)[j];
            clear_sprite_pieces(frame.pieces_00,destruction);
            read_count<SpriteDestructionPiece>(frame.pieces_00,stream,SpriteReadAllocationKind::pieces,0x48,hooks,[](auto&){});
            for(std::uint32_t k=0;k<frame.pieces_00.count_00;++k) {
                auto& storage=static_cast<SpriteDestructionPiece*>(frame.pieces_00.data_0c)[k];
                RuntimeSpritePiece piece;
                std::memcpy(&piece,storage.opaque_00.data(),sizeof(piece));
                // The real array already contains the numeric fields when
                // texture allocation runs. Publish the local typed view at
                // those boundaries without aliasing the opaque storage.
                PieceTextureBridge bridge{piece,storage,texture_hooks};
                const ResourceRegistrationHooks bridged_hooks{&bridge,
                    [](void* p,std::uint32_t bytes) {
                        auto& b=*static_cast<PieceTextureBridge*>(p);b.publish();
                        const auto result=b.hooks.allocate_49c1a8(b.hooks.context,bytes);b.reload();return result;
                    },[](void* p,ResourceCacheEntry* entries) {
                        auto& b=*static_cast<PieceTextureBridge*>(p);b.publish();
                        b.hooks.deallocate_49c13d(b.hooks.context,entries);b.reload();
                    }};
                read_runtime_sprite_piece(piece,stream,textures,bridged_hooks);
                std::memcpy(storage.opaque_00.data(),&piece,sizeof(piece));
            }
        }
        read_level_string(stream,clip.name_18.data(),128);
        const auto bits=double_bits(stream);std::memcpy(clip.speed_bits_10.data(),&bits,8);
    }
    clear_sprite_triple_arrays(resource.triple_arrays_10,destruction);
    read_count<SpriteDestructionTripleArray>(resource.triple_arrays_10,stream,SpriteReadAllocationKind::triple_arrays,8,hooks,
        [](auto& array){array.data_04=nullptr;});
    for(std::uint32_t i=0;i<resource.triple_arrays_10.count_00;++i) {
        auto& array=static_cast<SpriteDestructionTripleArray*>(resource.triple_arrays_10.data_0c)[i];
        array.type_00=static_cast<std::uint32_t>(integer(stream,4));
        if(array.data_04)hooks.free_memory(hooks.context,array.data_04);
        if(array.type_00==2)array.data_04=nullptr;
        else {
            const std::uint32_t triples=array.type_00==1?256:16;
            array.data_04=hooks.allocate(hooks.context,SpriteReadAllocationKind::triples,triples,triples*24);
            if(!array.data_04)throw std::bad_alloc();
            // 4011A0 invokes the empty 401B80 constructor; no element writes.
        }
        const auto triples=array.type_00==0?16u:array.type_00==1?256u:0u;
        for(std::uint32_t j=0;j<triples;++j)for(unsigned k=0;k<3;++k) {
            const auto bits=double_bits(stream);
            std::memcpy(static_cast<std::byte*>(array.data_04)+j*24+k*8,&bits,8);
        }
    }
}
}
