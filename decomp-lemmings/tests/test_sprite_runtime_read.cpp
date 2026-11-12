#include "sprite_runtime_read_fixture.hpp"
namespace {void require(bool p,const char* m){if(!p){std::cerr<<m<<'\n';std::exit(1);}}}
int main() {
    using namespace sprite_read_test;
    Fixture f(3);auto data=synthetic(3);f.run(data);
    require(f.cursor==data.size()&&f.resource.clips_00.count_00==3&&f.resource.clips_00.capacity_04==6,"growth must exceed requested for increments other than one");
    auto* clips=static_cast<SpriteDestructionClip*>(f.resource.clips_00.data_0c);
    for(unsigned i=3;i<6;++i) {
        require(clips[i].frames_00.count_00==0&&clips[i].frames_00.growth_08==1&&!clips[i].frames_00.data_0c,"construct capacity beyond logical count");
        require(clips[i].name_18[0]==0&&static_cast<unsigned char>(clips[i].name_18[1])==0xa5,"constructor only clears first name byte");
        require(clips[i].speed_bits_10[0]==std::byte{0xa5},"constructor preserves speed");
    }
    auto* triples=static_cast<SpriteDestructionTripleArray*>(f.resource.triple_arrays_10.data_0c);
    require(triples[0].type_00==3&&triples[0].data_04,"unknown type allocates sixteen triples");
    require(static_cast<std::byte*>(triples[0].data_04)[0]==std::byte{0xa5},"unknown type reads no triples");
    auto empty=synthetic(0);f.run(empty);
    require(!f.resource.clips_00.data_0c&&!f.resource.triple_arrays_10.data_0c,"repeat read clears old arrays");
    for(const auto& block:f.blocks)if(block.kind!=5)require(block.freed,"nested old allocation not freed");
    Fixture one;one.run(synthetic(1));
    auto* array=static_cast<SpriteDestructionTripleArray*>(one.resource.triple_arrays_10.data_0c);
    require(array[0].type_00==1&&array[0].data_04,"type one storage");
    std::uint64_t bits=0;std::memcpy(&bits,array[0].data_04,8);
    require(bits==0x7ff8000000000002ULL,"triple x87 quieting");
    std::cout<<"runtime SPR allocation, spare constructors, type branches and repeated read tests passed\n";
}
