#include "lemmings/texture_resource.hpp"
#include "lemmings/box_archive.hpp"
#include <iostream>

int main(int argc,char** argv) {
    std::uint64_t count=0,bytes=0;
    try {
        for(int i=1;i<argc;++i) {
            auto archive=lemmings::read_box_archive_index(argv[i]);
            for(std::size_t j=0;j<archive.entries.size();++j) {
                const auto& name=archive.entries[j].name;
                if(name.starts_with("TEXTURES\\") && name.ends_with(".BMP")) {
                    const auto decoded=lemmings::decode_texture_bitmap(archive.read_entry(j));
                    if(decoded.size()<54 || decoded[0]!='B' || decoded[1]!='M') {
                        std::cerr<<"invalid decoded BMP "<<name<<'\n';return 1;
                    }
                    ++count;bytes+=decoded.size();
                }
            }
        }
        std::cout<<count<<" installed texture bitmaps / "<<bytes<<" decoded bytes\n";
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
