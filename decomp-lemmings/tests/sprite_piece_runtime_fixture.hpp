#pragma once
#include "resource_registration_fixture.hpp"
#include "lemmings/sprite_piece_runtime.hpp"
#include <bit>

namespace sprite_piece_test {
using namespace lemmings;
struct Fixture:registration_test::Fixture {
    RuntimeSpritePiece piece;
    std::vector<std::uint8_t> bytes;
    std::vector<std::uint64_t> piece_trace;
    std::string name;
    void* global_manager=nullptr;
    std::size_t cursor=0;
    template<class T> void append(T value) {
        const auto old=bytes.size();bytes.resize(old+sizeof(value));
        std::memcpy(bytes.data()+old,&value,sizeof(value));
    }
    explicit Fixture(unsigned t,bool n=false):registration_test::Fixture(t,n) {
        std::memset(&piece,0xa5,sizeof(piece));
        name=input.data()+alignment;
        if(t%5==0 && state.count_30>0) {
            name=old_block.entries[0].name_00.data();
            old_block.entries[0].unknown_84|=0x80;
            if(state.count_30>2) {
                std::strcpy(old_block.entries[2].name_00.data(),name.c_str());
                old_block.entries[2].unknown_84|=1;
            }
        }
        if(t%5==1 && state.count_30>0) {
            std::strcpy(old_block.entries[0].name_00.data(),name.c_str());
            old_block.entries[0].unknown_84&=0xffffff00u;
        }
        const std::uint64_t specials[]={0,0x8000000000000000ULL,1,0x8000000000000001ULL,
            0x7fefffffffffffffULL,0xffefffffffffffffULL,0x7ff0000000000000ULL,
            0xfff0000000000000ULL,0x7ff0000000000001ULL,0xfff8000000001234ULL};
        std::uint64_t seed=0x485ce0+t;
        auto random=[&]{seed=seed*6364136223846793005ULL+1;return seed;};
        for(unsigned i=0;i<4;++i)append(static_cast<std::uint32_t>(random()));
        for(unsigned i=0;i<4;++i)append(t%3==0?specials[(t+i)%10]:random());
        for(unsigned i=0;i<4;++i)append(static_cast<std::uint32_t>(random()));
        switch(t%11) {
        case 0: append(std::int32_t{-1});name.clear();break;
        case 1: append(std::uint32_t{261});name.clear();break;
        case 2: append(std::uint32_t{260});
            for(unsigned i=0;i<260;++i)bytes.push_back(i<name.size()?static_cast<std::uint8_t>(name[i]):0);
            break;
        default:append(static_cast<std::uint32_t>(name.size()));bytes.insert(bytes.end(),name.begin(),name.end());
        }
        sync_to_raw();
    }
    void before_callback() {
        if(global_manager)std::memcpy(raw_manager.data(),global_manager,64);
        for(const auto x:piece.values_00)piece_trace.push_back(x);
        for(const auto x:piece.double_bits_10)piece_trace.push_back(x);
        for(const auto x:piece.values_30)piece_trace.push_back(x);
        piece_trace.push_back(static_cast<std::uint32_t>(piece.texture_id_40));
        piece_trace.push_back(piece.unknown_44);
    }
    void after_callback() {
        if(trial&16){piece.values_00[0]=0xabcdef12;piece.texture_id_40=-77;piece.unknown_44=0x12345678;}
        if(global_manager)std::memcpy(global_manager,raw_manager.data(),64);
    }
    ResourceEntryAllocation runtime_allocate(std::uint32_t size) {
        before_callback();auto result=allocate(size);after_callback();return result;
    }
    void runtime_deallocate(ResourceCacheEntry* entries) {
        before_callback();deallocate(entries);after_callback();
    }
    ResourceRegistrationHooks runtime_hooks() {
        return {this,[](void* p,std::uint32_t n){return static_cast<Fixture*>(p)->runtime_allocate(n);},
            [](void* p,ResourceCacheEntry* e){static_cast<Fixture*>(p)->runtime_deallocate(e);}};
    }
    void run_piece() {
        LevelByteStream stream{bytes};auto h=runtime_hooks();
        read_runtime_sprite_piece(piece,stream,state,h);cursor=stream.cursor;
    }
    bool same(Fixture& other) {
        return equals(other)&&trace==other.trace&&piece_trace==other.piece_trace&&
            std::memcmp(&piece,&other.piece,sizeof(piece))==0&&cursor==other.cursor;
    }
};
}
