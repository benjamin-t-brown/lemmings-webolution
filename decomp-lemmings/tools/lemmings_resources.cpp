#include "lemmings/box_archive.hpp"
#include "lemmings/resource_cache_load.hpp"
#include "lemmings/resource_cache_cleanup.hpp"
#include "lemmings/resource_registration.hpp"
#include "lemmings/resource_manager_construction.hpp"
#include "lemmings/sprite_factory.hpp"
#include "lemmings/sprite_resource.hpp"
#include "lemmings/sprite_runtime_read.hpp"
#include "lemmings/file_stream_runtime.hpp"
#include "lemmings/sprite_render_geometry.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string_view>

// Integration harness. Recovered cache/registration/factory source is used;
// storage, archive file access and parsed vectors are portable adapters. The
// vectors below do not claim the original nested runtime array memory layout.
namespace {
using namespace lemmings;
void require(bool condition,const char* message) {
    if(!condition)throw std::runtime_error(message);
}
struct EntryStorage {
    std::unique_ptr<ResourceCacheEntry[]> entries;
    std::uint32_t cookie=0;
};
struct LoadedSprite {
    SpriteFactoryResource runtime;
    SpriteResource decoded;
    std::vector<AnimationClip> clips;
    AnimationResource animation;
};
struct OpenFile {
    FileStreamRuntime stream;
    std::vector<std::uint8_t> bytes;
    std::size_t backend_cursor=0;
};
struct FactoryTerminated {};
struct RuntimeStorage {
    std::unique_ptr<std::byte[]> owner;
    std::size_t bytes;
};
class Harness {
public:
    std::vector<BoxArchiveIndex> archives;
    ResourceCacheLookupManager manager;
    std::uint32_t loaded_count=0;
    ResourceCache cache;
    ResourceRegistration registration{cache,0,0,1};
    ResourceCache texture_cache;
    ResourceRegistration textures{texture_cache,0,0,1};
    std::map<ResourceCacheEntry*,std::unique_ptr<EntryStorage>> entry_storage;
    std::map<ResourceCacheLookupNode*,std::unique_ptr<ResourceCacheLookupNode>> nodes;
    std::map<SpriteFactoryResource*,std::unique_ptr<LoadedSprite>> sprites;
    std::map<SpriteFactoryFile*,OpenFile> files;
    std::map<void*,RuntimeStorage> runtime_storage;
    std::string factory_error;
    unsigned created=0,destroyed=0,notifications=0,animation_checks=0;
    unsigned stream_reads=0,stream_closes=0;
    unsigned quads_checked=0;
    unsigned stream_seeks=0;
    FileStreamBufferPointers shared_buffer;
    std::array<std::byte,1> buffer_storage;
    explicit Harness(std::vector<BoxArchiveIndex> sources):archives(std::move(sources)) {
        construct_sprite_resource_manager(manager,loaded_count,registration);
        construct_resource_registration(textures);
    }

    void free_runtime(void* allocation) {
        require(runtime_storage.erase(allocation)==1,"unknown runtime sprite allocation");
    }
    SpriteRuntimeReadHooks runtime_read_hooks() {
        return {this,
            [](void* p,SpriteReadAllocationKind kind,std::uint32_t count,std::uint32_t original_bytes)->void* {
                auto& h=*static_cast<Harness*>(p);
                std::size_t stride=0;std::uint32_t original_stride=0;
                switch(kind) {
                case SpriteReadAllocationKind::clips:stride=sizeof(SpriteDestructionClip);original_stride=0x98;break;
                case SpriteReadAllocationKind::frames:stride=sizeof(SpriteDestructionFrame);original_stride=0x10;break;
                case SpriteReadAllocationKind::pieces:stride=sizeof(SpriteDestructionPiece);original_stride=0x48;break;
                case SpriteReadAllocationKind::triple_arrays:stride=sizeof(SpriteDestructionTripleArray);original_stride=8;break;
                case SpriteReadAllocationKind::triples:stride=24;original_stride=24;break;
                }
                const bool cookie=kind!=SpriteReadAllocationKind::triples;
                require(original_bytes==count*original_stride+(cookie?4u:0u),"runtime sprite allocation size");
                const auto bytes=static_cast<std::size_t>(count)*stride;
                const auto alignment=alignof(std::max_align_t);
                auto owner=std::make_unique<std::byte[]>(bytes+alignment+4);
                std::memset(owner.get(),0xa5,bytes+alignment+4);
                void* elements=owner.get()+(cookie?4:0);
                std::size_t space=bytes+alignment;
                require(std::align(alignment,bytes,elements,space)!=nullptr,"runtime allocation alignment");
                void* allocation=static_cast<std::byte*>(elements)-(cookie?4:0);
                h.runtime_storage.emplace(allocation,RuntimeStorage{std::move(owner),bytes});
                return elements;
            },
            [](void* p,void* allocation){static_cast<Harness*>(p)->free_runtime(allocation);}};
    }

    ResourceRegistrationHooks registration_hooks() {
        return {this,
            [](void* p,std::uint32_t bytes) {
                auto& h=*static_cast<Harness*>(p);
                require(bytes>=4 && (bytes-4)%0x88==0,"invalid original entry allocation size");
                const auto count=(bytes-4)/0x88;
                auto storage=std::make_unique<EntryStorage>();
                storage->entries=std::make_unique<ResourceCacheEntry[]>(count);
                auto* entries=storage->entries.get();auto* cookie=&storage->cookie;
                h.entry_storage.emplace(entries,std::move(storage));
                return ResourceEntryAllocation{entries,cookie};
            },
            [](void* p,ResourceCacheEntry* entries) {
                require(static_cast<Harness*>(p)->entry_storage.erase(entries)==1,"unknown entry allocation");
            }};
    }
    FileStreamRuntimeHooks file_hooks() {
        return {this,
            [](void* p,void* destination,std::uint32_t requested,void* file) {
                auto& h=*static_cast<Harness*>(p);auto& state=*static_cast<OpenFile*>(file);
                const auto count=std::min<std::size_t>(requested,state.bytes.size()-state.backend_cursor);
                if(count)std::memcpy(destination,state.bytes.data()+state.backend_cursor,count);
                state.backend_cursor+=count;++h.stream_reads;
                return static_cast<std::uint32_t>(count);
            },
            [](void* p,void*){++static_cast<Harness*>(p)->stream_closes;return 0;}};
    }
    SpriteFactoryHooks sprite_hooks() {
        return {this,
            [](void* p,std::size_t size) {
                auto& h=*static_cast<Harness*>(p);require(size==32,"sprite allocation size");
                auto sprite=std::make_unique<LoadedSprite>();auto* result=&sprite->runtime;
                h.sprites.emplace(result,std::move(sprite));++h.created;return result;
            },
            [](void*,char* output,const char* format,const char* name) {
                const int count=std::snprintf(output,260,format,name);
                require(count>=0 && count<260,"sprite path exceeds original buffer");
            },
            [](void* p,SpriteFactoryFile& file,const char* path) {
                auto& h=*static_cast<Harness*>(p);auto [iterator,inserted]=h.files.try_emplace(&file);
                require(inserted,"duplicate file object");
                construct_named_file_stream(iterator->second.stream,path,{});
            },
            [](void* p,SpriteFactoryFile& file,std::uint32_t mode,std::uint32_t flag) {
                auto& h=*static_cast<Harness*>(p);require(mode==1 && flag==0,"sprite open arguments");
                auto& state=h.files.at(&file);
                const auto location=find_box_entry(h.archives,state.stream.name_14.data());
                if(!location)return 0u;
                state.bytes=h.archives[location->archive].read_entry(location->entry);
                state.stream.file_118=&state;state.stream.size_10=static_cast<std::uint32_t>(state.bytes.size());
                state.stream.mode_130=1;state.stream.archive_11d=1;
                state.stream.archive_offset_124=h.archives[location->archive].entries[location->entry].offset;
                state.stream.archive_index_128=static_cast<std::uint32_t>(location->archive);
                return static_cast<std::uint32_t>(finish_archive_file_stream_open(state.stream,h.shared_buffer,h.buffer_storage.data(),
                    {&h,
                        [](void*,void* file){auto& f=*static_cast<OpenFile*>(file);return static_cast<std::int32_t>(f.stream.archive_offset_124+f.backend_cursor);},
                        [](void* context,void* file,std::int32_t offset,std::int32_t origin) {
                            auto& owner=*static_cast<Harness*>(context);auto& f=*static_cast<OpenFile*>(file);
                            require(origin==0,"archive finalization seek origin");
                            const auto position=static_cast<std::uint32_t>(offset)-f.stream.archive_offset_124;
                            require(position<=f.bytes.size(),"archive finalization seek position");
                            f.backend_cursor=position;++owner.stream_seeks;return 0;
                        }}));
            },
            [](void* p,SpriteFactoryResource* resource,SpriteFactoryFile& file) {
                auto& h=*static_cast<Harness*>(p);
                // Malformed input is reported after recovered normal-return
                // factory cleanup. Exceptions do not cross its read boundary.
                try {
                    auto& sprite=*h.sprites.at(resource);
                    auto& state=h.files.at(&file);std::vector<std::uint8_t> bytes(state.bytes.size());
                    const auto actual=read_file_stream(state.stream,static_cast<std::uint32_t>(bytes.size()),bytes.data(),h.file_hooks());
                    require(actual==bytes.size() && state.stream.cursor_120==actual,"sprite block read cursor");
                    sprite.decoded=parse_sprite_resource(bytes);
                    // The serialized texture field accepts 259 bytes, but
                    // original registration copies into a 128-byte slot.
                    // Keep invalid inputs outside that recovered API's domain.
                    for(const auto& clip:sprite.decoded.clips)
                        for(const auto& frame:clip.frames)
                            for(const auto& piece:frame.pieces)
                                require(piece.texture_name.size()<128,
                                    "texture name exceeds original registration slot buffer");
                    LevelByteStream stream{bytes};
                    read_runtime_sprite_resource(*resource,stream,h.textures,h.registration_hooks(),h.runtime_read_hooks());
                    require(stream.cursor==stream.bytes.size(),"runtime sprite reader trailing bytes");
                    sprite.clips=sprite_animation_clips(sprite.decoded);
                    sprite.animation.clips=sprite.clips;
                    h.validate_runtime(sprite);
                } catch(const std::exception& error) {h.factory_error=error.what();}
            },
            [](void* p,SpriteFactoryFile& file) {
                auto& h=*static_cast<Harness*>(p);auto& state=h.files.at(&file);
                close_file_stream(state.stream,h.file_hooks());state.bytes.clear();
            },
            [](void* p,SpriteFactoryFile& file) {
                auto& h=*static_cast<Harness*>(p);auto& state=h.files.at(&file);
                destroy_file_stream(state.stream,h.file_hooks());
                require(!state.stream.file_118 && state.stream.cursor_120==0 && !state.stream.opened_12c,"file close state");
                require(h.files.erase(&file)==1,"sprite file lifetime");
            },
            [](void* p,const char*,std::uint32_t line,const char* path) {
                require(line==31,"sprite diagnostic line");
                static_cast<Harness*>(p)->factory_error="cannot open "+std::string(path);
            },
            [](void*,int status) {
                require(status==0,"original sprite exit status");throw FactoryTerminated{};
            }};
    }
    SpriteFactoryDestroyHooks destroy_hooks() {
        return {this,
            [](void* p,SpriteRuntimeArray& array) {
                auto& h=*static_cast<Harness*>(p);
                clear_sprite_clips(array,{&h,[](void* context,void* allocation){static_cast<Harness*>(context)->free_runtime(allocation);}});
            },
            [](void* p,SpriteRuntimeArray& array) {
                auto& h=*static_cast<Harness*>(p);
                clear_sprite_triple_arrays(array,{&h,[](void* context,void* allocation){static_cast<Harness*>(context)->free_runtime(allocation);}});
            },
            [](void* p,SpriteFactoryResource* resource) {
                auto& h=*static_cast<Harness*>(p);
                require(h.sprites.erase(resource)==1,"unknown sprite allocation");++h.destroyed;
            }};
    }
    ResourceCacheLoadHooks load_hooks() {
        return {this,
            [](void* p,std::int32_t id) {
                auto& h=*static_cast<Harness*>(p);
                return create_resource_by_index(h.cache,id,{&h,
                    [](void* context,const char* name)->void* {
                        auto& backend=*static_cast<Harness*>(context);
                        return create_sprite_resource(name,backend.sprite_hooks());
                    }});
            },
            [](void* p) {
                auto& h=*static_cast<Harness*>(p);auto node=std::make_unique<ResourceCacheLookupNode>();
                auto* result=node.get();h.nodes.emplace(result,std::move(node));return result;
            }};
    }
    ResourceCacheHooks cache_hooks() {
        return {this,
            [](void* p,std::int32_t id){return find_cached_resource(static_cast<Harness*>(p)->manager,id);},
            [](void* p,std::int32_t id) {
                auto& h=*static_cast<Harness*>(p);
                return load_cached_resource(h.manager,h.loaded_count,id,h.load_hooks());
            },
            [](void* p,const char*){++static_cast<Harness*>(p)->notifications;}};
    }
    ResourceCacheCleanupHooks cleanup_hooks() {
        return {this,
            [](void* p,std::int32_t id){auto& h=*static_cast<Harness*>(p);auto callbacks=h.load_hooks();return callbacks.create(callbacks.context,id);},
            [](void* p,void* resource){auto& h=*static_cast<Harness*>(p);destroy_sprite_resource(static_cast<SpriteFactoryResource*>(resource),h.destroy_hooks());},
            [](void* p,ResourceCacheLookupNode* node){require(static_cast<Harness*>(p)->nodes.erase(node)==1,"unknown cache node");}};
    }
    SpriteFactoryResource* acquire(std::int32_t id) {
        SpriteFactoryResource* result;
        try {result=static_cast<SpriteFactoryResource*>(acquire_resource(cache,id,cache_hooks()));}
        catch(const FactoryTerminated&) {throw std::runtime_error(factory_error);}
        if(!factory_error.empty())throw std::runtime_error(factory_error);
        require(result && files.empty(),"incomplete sprite factory load");return result;
    }
    void check_animation(std::int32_t id,LoadedSprite& sprite) {
        const auto clip=std::find_if(sprite.clips.begin(),sprite.clips.end(),[](const auto& c){
            return c.frame_count>0 && std::isfinite(c.speed) && c.speed>0 && c.speed<1000;
        });
        if(clip==sprite.clips.end())return;
        AnimationPlayback playback;
        AnimationResourceHooks hooks{this,
            [](void* p,std::int32_t resource_id)->const AnimationResource* {
                auto& h=*static_cast<Harness*>(p);return &h.sprites.at(h.acquire(resource_id))->animation;
            },
            [](void* p,std::int32_t resource_id){release_resource(static_cast<Harness*>(p)->cache,resource_id);}};
        bind_animation_resource(playback,id,hooks);
        select_animation(playback,clip->name.c_str());
        advance_animation(playback,0.125,hooks);
        require(playback.frame_1cc>=0 && playback.frame_1cc<clip->frame_count,"animation frame outside clip");
        hooks.release(hooks.context,id);++animation_checks;
    }
    void check_geometry(LoadedSprite& sprite) {
        SpriteRenderGeometry geometry{};geometry.resource_1f8=&sprite.runtime;
        auto* clips=static_cast<const SpriteDestructionClip*>(sprite.runtime.clips_00.data_0c);
        for(std::uint32_t i=0;i<sprite.runtime.clips_00.count_00;++i) {
            geometry.clip_index_1c8=static_cast<std::int32_t>(i);
            auto* frames=static_cast<const SpriteDestructionFrame*>(clips[i].frames_00.data_0c);
            for(std::uint32_t j=0;j<clips[i].frames_00.count_00;++j) {
                geometry.frame_index_1cc=static_cast<std::int32_t>(j);
                for(std::uint32_t k=0;k<frames[j].pieces_00.count_00;++k) {
                    geometry.piece_index_1b8=static_cast<std::int32_t>(k);
                    // Chosen harness inputs exercise the normal scale branch.
                    // They are not a recovered gameplay camera or model pose.
                    build_sprite_rectangle(geometry,0,0,32,32,0,0,{this,
                        [](void* p,SpriteRenderGeometry& quad) {
                            auto& h=*static_cast<Harness*>(p);
                            require(quad.texture_id_1a8>=0 && quad.texture_id_1a8<h.textures.count_30,"quad texture binding");
                            for(const auto& vertex:quad.vertices_08)
                                require(std::isfinite(vertex.x_18) && std::isfinite(vertex.y_20),"nonfinite sprite rectangle");
                            ++h.quads_checked;
                        }});
                }
            }
        }
    }
    void invalidate_and_unload() {
        invalidate_and_unload_resources(manager,loaded_count,cache,registration.count_30,cleanup_hooks());
    }
    void validate_runtime(const LoadedSprite& sprite) const {
        auto quiet=[](std::uint64_t bits) {
            if((bits&0x7ff0000000000000ULL)==0x7ff0000000000000ULL && (bits&0xfffffffffffffULL))bits|=0x8000000000000ULL;
            return bits;
        };
        const auto& runtime=sprite.runtime;
        require(runtime.clips_00.count_00==sprite.decoded.clips.size(),"runtime clip count");
        auto* clips=static_cast<const SpriteDestructionClip*>(runtime.clips_00.data_0c);
        for(std::size_t i=0;i<sprite.decoded.clips.size();++i) {
            const auto& expected=sprite.decoded.clips[i];const auto& clip=clips[i];
            std::uint64_t speed;std::memcpy(&speed,clip.speed_bits_10.data(),8);
            require(speed==quiet(expected.speed_bits_10) && expected.name==clip.name_18.data(),"runtime clip fields");
            require(clip.frames_00.count_00==expected.frames.size(),"runtime frame count");
            auto* frames=static_cast<const SpriteDestructionFrame*>(clip.frames_00.data_0c);
            for(std::size_t j=0;j<expected.frames.size();++j) {
                const auto& expected_frame=expected.frames[j];const auto& frame=frames[j];
                require(frame.pieces_00.count_00==expected_frame.pieces.size(),"runtime piece count");
                auto* pieces=static_cast<const SpriteDestructionPiece*>(frame.pieces_00.data_0c);
                for(std::size_t k=0;k<expected_frame.pieces.size();++k) {
                    const auto& expected_piece=expected_frame.pieces[k];RuntimeSpritePiece piece;
                    std::memcpy(&piece,pieces[k].opaque_00.data(),sizeof(piece));
                    require(piece.values_00==expected_piece.values_00 && piece.values_30==expected_piece.values_30,"runtime piece words");
                    for(unsigned n=0;n<4;++n)require(piece.double_bits_10[n]==quiet(expected_piece.double_bits_10[n]),"runtime piece doubles");
                    require(piece.unknown_44==0xa5a5a5a5,"runtime piece changed unknown field");
                    require(piece.texture_id_40>=0 && piece.texture_id_40<textures.count_30,"runtime texture ID");
                    require(expected_piece.texture_name==texture_cache.entries_30[piece.texture_id_40].name_00.data(),"runtime texture name resolution");
                }
            }
        }
        require(runtime.triple_arrays_10.count_00==sprite.decoded.triple_arrays.size(),"runtime triple-array count");
        auto* arrays=static_cast<const SpriteDestructionTripleArray*>(runtime.triple_arrays_10.data_0c);
        for(std::size_t i=0;i<sprite.decoded.triple_arrays.size();++i) {
            const auto& expected=sprite.decoded.triple_arrays[i];const auto& array=arrays[i];
            require(array.type_00==expected.type,"runtime triple-array type");
            for(std::size_t j=0;j<expected.triples.size();++j)for(unsigned k=0;k<3;++k) {
                std::uint64_t bits;std::memcpy(&bits,static_cast<const std::byte*>(array.data_04)+j*24+k*8,8);
                require(bits==quiet(expected.triples[j][k]),"runtime triple values");
            }
        }
    }
};

std::string uppercase(std::string name) {
    for(auto& c:name)if(c>='a' && c<='z')c-=32;
    return name;
}
std::set<std::string> sprite_names(const std::vector<BoxArchiveIndex>& archives) {
    std::set<std::string> names;
    for(const auto& archive:archives)for(const auto& entry:archive.entries) {
        auto name=uppercase(entry.name);
        if(name.starts_with("SPRITES\\") && name.ends_with(".SPR"))names.insert(name.substr(8,name.size()-12));
    }
    return names;
}
int run(int argc,char** argv) {
    std::string base,patch,sprite_name;bool all=false;
    for(int i=1;i<argc;++i) {
        const std::string option=argv[i];
        if(option=="--help") {
            std::cout<<"Usage: lemmings_resources --archive <lemmings.box> [--patch <patch.box>] (--all | --sprite <name>)\n";
            return 0;
        }
        if(option=="--all") {all=true;continue;}
        std::string* destination=option=="--archive"?&base:option=="--patch"?&patch:option=="--sprite"?&sprite_name:nullptr;
        if(!destination || ++i==argc || !destination->empty())throw std::invalid_argument("invalid option: "+option);
        *destination=argv[i];
    }
    require(!base.empty() && all!=!sprite_name.empty(),"provide --archive and exactly one of --all or --sprite");
    std::vector<BoxArchiveIndex> archives;
    if(!patch.empty())archives.push_back(read_box_archive_index(patch));
    archives.push_back(read_box_archive_index(base));
    auto names=all?sprite_names(archives):std::set<std::string>{sprite_name};
    require(!names.empty(),"no sprite resources found");
    Harness h(std::move(archives));
    std::vector<std::int32_t> ids;
    std::size_t clips=0,frames=0,pieces=0;
    for(const auto& name:names) {
        require(name.size()<128,"resource name exceeds original slot buffer");
        ids.push_back(register_resource_slot(h.registration,name.c_str(),h.registration_hooks()));
    }
    for(auto id:ids) {
        auto* first=h.acquire(id);const auto creations=h.created;
        require(h.acquire(id)==first && h.created==creations,"cache hit created another sprite");
        auto& loaded=*h.sprites.at(first);clips+=loaded.decoded.clips.size();
        for(const auto& clip:loaded.decoded.clips)for(const auto& frame:clip.frames) {
            ++frames;pieces+=frame.pieces.size();
        }
        h.check_animation(id,loaded);
        h.check_geometry(loaded);
        release_resource(h.cache,id);release_resource(h.cache,id);
        require(h.cache.entries_30[id].cached_80->references_08==0,"unbalanced resource references");
    }
    require(h.loaded_count==names.size(),"loaded count after cache fill");
    // Retain one node across unload/recreate; all zero-reference nodes evict.
    h.acquire(ids.front());h.invalidate_and_unload();
    require(h.loaded_count==0 && h.sprites.empty() && h.nodes.size()==1,"unload retained-node state");
    recreate_and_refresh_resources(h.manager,h.loaded_count,h.cache,h.registration.count_30,h.cleanup_hooks());
    require(h.loaded_count==1 && h.cache.entries_30[ids.front()].cached_80,"recreate/cache refresh");
    release_resource(h.cache,ids.front());h.invalidate_and_unload();
    // Loading after eviction must allocate again and rebind the slot cache.
    h.acquire(ids.front());release_resource(h.cache,ids.front());h.invalidate_and_unload();
    require(h.loaded_count==0 && h.nodes.empty() && h.sprites.empty() && h.files.empty() && h.runtime_storage.empty(),"resource cleanup incomplete");
    for(auto id:ids)h.cache.entries_30[id].unknown_84|=0xa5a5a500;
    clear_unused_resource_flags_and_evict(h.manager,h.loaded_count,h.cache,h.registration.count_30,h.cleanup_hooks());
    for(auto id:ids)require(h.cache.entries_30[id].unknown_84==0xa5a5a500,"unused flag clearing changed unknown bytes");
    const auto old_count=h.registration.count_30;
    require(register_resource_slot(h.registration,names.begin()->c_str(),h.registration_hooks())==0 && h.registration.count_30==old_count,"first unused slot was not reused");
    std::cout<<"{\"resources\":"<<names.size()<<",\"clips\":"<<clips<<",\"frames\":"<<frames
        <<",\"pieces\":"<<pieces<<",\"created\":"<<h.created<<",\"destroyed\":"<<h.destroyed
        <<",\"animation_checks\":"<<h.animation_checks<<",\"loaded_after_cleanup\":"<<h.loaded_count
        <<",\"texture_names\":"<<h.textures.count_30
        <<",\"stream_reads\":"<<h.stream_reads<<",\"stream_closes\":"<<h.stream_closes
        <<",\"stream_seeks\":"<<h.stream_seeks
        <<",\"quads_checked\":"<<h.quads_checked
        <<",\"slot_reused\":true}\n";
    return 0;
}
} // namespace
int main(int argc,char** argv) {
    try {return run(argc,argv);}
    catch(const FactoryTerminated&) {std::cerr<<"lemmings_resources: original factory terminated after an open failure\n";return 1;}
    catch(const std::exception& error) {std::cerr<<"lemmings_resources: "<<error.what()<<'\n';return 1;}
}
