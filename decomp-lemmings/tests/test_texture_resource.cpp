#include "lemmings/texture_resource.hpp"
#include <cstdlib>
#include <iostream>
#include <stdexcept>

namespace {
void require(bool value) { if(!value)std::abort(); }
struct Writer {
    std::vector<std::uint8_t> bytes{0,0,0};
    unsigned used=0;
    void bit(unsigned value) {
        if(used==0)bytes.push_back(0);
        bytes.back()|=static_cast<std::uint8_t>(value<<(7-used));
        used=(used+1)%8;
    }
    void bits(unsigned value,unsigned count) { while(count--)bit((value>>count)&1); }
    void integer(unsigned value) {
        const auto shifted=value+1;
        unsigned length=0;
        for(auto n=shifted;n>1;n>>=1)++length;
        for(unsigned i=0;i<length;++i)bit(1);
        bit(0);bits(shifted,length);
    }
};
template<class Call>void rejects(Call call) {
    try {call();std::abort();}catch(const std::invalid_argument&){}
}
}
int main() {
    using namespace lemmings;
    unsigned cases=0;
    for(unsigned length=1;length<=1024;++length) {
        Writer stream;stream.integer(length);
        std::vector<std::uint8_t> expected;
        for(unsigned i=0;i<length;++i){const auto byte=static_cast<std::uint8_t>(i*73+length);expected.push_back(byte);stream.bits(byte,8);}
        require(decode_texture_block(stream.bytes,length)==expected);++cases;
    }
    for(unsigned length=2;length<=1024;++length) {
        Writer stream;stream.integer(1);stream.bits(0xa5,8);stream.integer(length-2);stream.integer(0);
        require(decode_texture_block(stream.bytes,length+1)==std::vector<std::uint8_t>(length+1,0xa5));++cases;
    }
    Writer huffman;huffman.bytes={0x11,0x22,0x11,0x22,0x11,0x22};
    huffman.bits(6,3);huffman.bits('A',8);huffman.bits('B',8);huffman.bits(6,3);huffman.bits(2,2);
    const std::vector<std::uint8_t> ab={'A','B','A','B','A','B'};
    require(decode_texture_block(huffman.bytes,6)==ab);++cases;
    const std::vector<std::uint8_t> raw={'B','M',0,1,2,3};
    require(decode_texture_bitmap(raw)==raw);
    const std::vector<std::uint8_t> packed={2,7,0,2,0,3,'B','M',9,0,4,0,3,0,1,2,3,0xff};
    require(decode_texture_bitmap(packed)==raw);
    require(decode_texture_bitmap(std::vector<std::uint8_t>{0,0xff}).empty());
    rejects([]{decode_texture_bitmap({});});
    rejects([]{decode_texture_bitmap(std::vector<std::uint8_t>{1,4,0,0,0,3});});
    rejects([]{decode_texture_bitmap(std::vector<std::uint8_t>{1,5,0,0,0,7});});
    rejects([]{decode_texture_bitmap(std::vector<std::uint8_t>{1,6,0,2,0,3,1});});
    rejects([]{decode_texture_block(std::vector<std::uint8_t>{0,0,0},0);});
    rejects([]{decode_texture_block(std::vector<std::uint8_t>{0x10},1);});
    rejects([]{decode_texture_block(std::vector<std::uint8_t>{0x11,0x11,0,0},1);});
    rejects([]{decode_texture_block(std::vector<std::uint8_t>{0,0,0,0},1);});
    std::cout<<cases<<" literal/backreference/Huffman unit cases plus envelope/malformed cases pass\n";
}
