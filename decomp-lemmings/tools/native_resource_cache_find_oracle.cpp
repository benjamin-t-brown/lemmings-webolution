#include "../tests/resource_cache_find_fixture.hpp"
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
int main(){
    static_assert(sizeof(void*)==4);
    _setmode(_fileno(stdin),_O_BINARY);
    auto* image=VirtualAlloc(reinterpret_cast<void*>(0x400000),0xc0000,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
    if(image!=reinterpret_cast<void*>(0x400000))return 3;
    std::uint32_t count=0;std::cin.read(reinterpret_cast<char*>(&count),4);if(count!=3)return 2;
    while(count--){std::uint32_t address,size;std::cin.read(reinterpret_cast<char*>(&address),4);std::cin.read(reinterpret_cast<char*>(&size),4);if(address<0x400000||address>0x4c0000||size>0x4c0000-address)return 2;std::cin.read(reinterpret_cast<char*>(address),size);}
    if(!std::cin)return 2;DWORD old;if(!VirtualProtect(image,0xc0000,PAGE_EXECUTE_READ,&old))return 2;FlushInstructionCache(GetCurrentProcess(),image,0xc0000);
    using Node=lemmings::ResourceCacheLookupNode;
    using Manager=lemmings::ResourceCacheLookupManager;
    using List=lemmings::ResourceCacheLookupList;
    using Find=Node*(__attribute__((thiscall))*)(const Manager*,std::int32_t);
    using First=Node*(__attribute__((thiscall))*)(const List*);
    using IsEnd=std::uint32_t(__attribute__((thiscall))*)(const List*,const Node*);
    auto find=reinterpret_cast<Find>(0x4306f0);
    auto first=reinterpret_cast<First>(0x430760);
    auto is_end=reinterpret_cast<IsEnd>(0x430780);
    std::size_t comparisons=0,helper_comparisons=0;
    for(unsigned trial=0;trial<20000;++trial){
        cache_find_test::Fixture fixture(trial);const auto original=fixture.bytes();auto& list=fixture.manager.list_04;
        if(first(&list)!=lemmings::resource_cache_first(list))return 1;++helper_comparisons;
        const Node* candidates[]={nullptr,&list.begin,&list.end,&fixture.nodes[0],reinterpret_cast<const Node*>(std::uintptr_t(1))};
        for(auto* candidate:candidates){if(((is_end(&list,candidate)&0xff)!=0)!=lemmings::resource_cache_is_end(list,candidate))return 1;++helper_comparisons;}
        for(auto id:fixture.queries){auto* expected=fixture.expected(id);auto* original_result=find(&fixture.manager,id);auto* recovered=lemmings::find_cached_resource_node(list,id);
            if(original_result!=expected||recovered!=expected||lemmings::find_cached_resource(fixture.manager,id)!=(expected?&expected->value:nullptr)||fixture.bytes()!=original){std::cerr<<"lookup mismatch trial "<<trial<<" id "<<id<<'\n';return 1;}++comparisons;}
    }
    VirtualFree(image,0,MEM_RELEASE);
    std::cout<<comparisons<<" resource-cache lookups and "<<helper_comparisons<<" helper calls match unchanged original code\n";
}
