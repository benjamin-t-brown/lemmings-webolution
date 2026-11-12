#include "lemmings/box_archive.hpp"
#include "lemmings/texture_resource.hpp"
#include <iostream>
#include <map>
#include <stdexcept>
#include <string_view>

namespace {
using namespace lemmings;
struct Totals {
    std::size_t bitmaps=0,packed=0,codec3=0,codec4=0;
    std::uint64_t input_bytes=0,output_bytes=0;
};
std::string uppercase(std::string name) {
    for(auto& c:name)if(c>='a' && c<='z')c-=32;
    return name;
}
std::uint16_t u16(std::span<const std::uint8_t> bytes,std::size_t at) {
    if(at>bytes.size() || bytes.size()-at<2)throw std::invalid_argument("truncated texture envelope");
    return static_cast<std::uint16_t>(bytes[at]|(std::uint16_t(bytes[at+1])<<8));
}
void inspect(const std::vector<std::uint8_t>& bytes,Totals& totals) {
    const auto decoded=decode_texture_bitmap(bytes);
    if(decoded.size()<54 || decoded[0]!='B' || decoded[1]!='M')
        throw std::invalid_argument("decoded texture is not a complete BMP header");
    ++totals.bitmaps;totals.input_bytes+=bytes.size();totals.output_bytes+=decoded.size();
    if(bytes.size()>=2 && bytes[0]=='B' && bytes[1]=='M')return;
    ++totals.packed;std::size_t cursor=1;
    for(unsigned i=0;i<bytes[0];++i) {
        const auto size=u16(bytes,cursor);
        if(size<5 || size>bytes.size()-cursor)throw std::invalid_argument("invalid texture block size");
        if(bytes[cursor+4]==3)++totals.codec3;
        else if(bytes[cursor+4]==4)++totals.codec4;
        cursor+=size;
    }
}
int run(int argc,char** argv) {
    std::string base,patch,entry_name;bool all=false;
    for(int i=1;i<argc;++i) {
        const std::string option=argv[i];
        if(option=="--help") {
            std::cout<<"Usage: lemmings_textures --archive <lemmings.box> [--patch <patch.box>] (--all | --entry <name.bmp>)\n";
            return 0;
        }
        if(option=="--all") {all=true;continue;}
        std::string* destination=option=="--archive"?&base:option=="--patch"?&patch:option=="--entry"?&entry_name:nullptr;
        if(!destination || ++i==argc || !destination->empty())throw std::invalid_argument("invalid option: "+option);
        *destination=argv[i];
    }
    if(base.empty() || all==!entry_name.empty())throw std::invalid_argument("provide --archive and exactly one of --all or --entry");
    std::vector<BoxArchiveIndex> archives;
    if(!patch.empty())archives.push_back(read_box_archive_index(patch));
    archives.push_back(read_box_archive_index(base));
    Totals totals;
    if(all) {
        std::map<std::string,BoxArchiveLocation> locations;
        for(std::size_t i=0;i<archives.size();++i)for(std::size_t j=0;j<archives[i].entries.size();++j) {
            auto name=uppercase(archives[i].entries[j].name);
            if(name.ends_with(".BMP"))locations.try_emplace(std::move(name),BoxArchiveLocation{i,j});
        }
        if(locations.empty())throw std::invalid_argument("no BMP entries found");
        for(const auto& [name,location]:locations) {
            try {inspect(archives[location.archive].read_entry(location.entry),totals);}
            catch(const std::exception& error){throw std::runtime_error(name+": "+error.what());}
        }
    } else {
        const auto location=find_box_entry(archives,entry_name);
        if(!location)throw std::invalid_argument("bitmap entry not found: "+entry_name);
        inspect(archives[location->archive].read_entry(location->entry),totals);
    }
    std::cout<<"{\"bitmaps\":"<<totals.bitmaps<<",\"packed_bitmaps\":"<<totals.packed
        <<",\"packed_bytes\":"<<totals.input_bytes<<",\"decoded_bytes\":"<<totals.output_bytes
        <<",\"codec3_blocks\":"<<totals.codec3<<",\"codec4_blocks\":"<<totals.codec4<<"}\n";
    return 0;
}
}
int main(int argc,char** argv) {
    try {return run(argc,argv);}
    catch(const std::exception& error){std::cerr<<"lemmings_textures: "<<error.what()<<'\n';return 1;}
}
