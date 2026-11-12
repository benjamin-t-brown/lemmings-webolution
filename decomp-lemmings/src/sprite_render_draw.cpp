#include "lemmings/sprite_render_draw.hpp"
#include <bit>
#include <cstring>
#include <cmath>
namespace lemmings {
namespace {
double attribute(const SpriteRenderVertex& v,unsigned offset){double d;std::memcpy(&d,reinterpret_cast<const std::byte*>(&v)+offset,8);return d;}
float rounded(long double d){volatile float f=static_cast<float>(d);return f;}
std::uint32_t integer(double d){
    if(!std::isfinite(d)||static_cast<long double>(d)<-9223372036854775808.0L||static_cast<long double>(d)>=9223372036854775808.0L)return 0;
    return static_cast<std::uint32_t>(static_cast<std::int64_t>(d));
}
}
void initialize_sprite_device_vertices(SpriteDeviceVertexArray& a){a={0,0,1,nullptr};}
void resize_sprite_device_vertices(SpriteDeviceVertexArray& a,std::int32_t count,const SpriteDrawHooks& h){
    if(a.capacity_04<count){
        if(a.growth_08==1)a.capacity_04=count;
        else do{a.capacity_04=std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(a.capacity_04)+static_cast<std::uint32_t>(a.growth_08));}while(a.capacity_04<=count);
        if(a.capacity_04>0){
            auto* p=h.allocate_49c1a8(h.context,static_cast<std::uint32_t>(a.capacity_04)*32);
            if(a.data_0c){for(std::int32_t i=0;i<a.count_00;++i)std::memcpy(p+i,a.data_0c+i,32);h.free_49c13d(h.context,a.data_0c);}
            a.data_0c=p;
        }else{if(a.data_0c)h.free_49c13d(h.context,a.data_0c);a.data_0c=nullptr;}
    }
    a.count_00=count;
}
void clear_sprite_device_vertices(SpriteDeviceVertexArray& a,const SpriteDrawHooks& h){if(a.data_0c){a.capacity_04=0;h.free_49c13d(h.context,a.data_0c);a.data_0c=nullptr;}a.count_00=0;}
void draw_sprite_polygon(SpriteVertexPointerArray& input,SpriteDeviceState& device,SpriteDrawState& s,const SpriteDrawHooks& h){
    if((s.initialized_flags_58d550&1)==0){s.initialized_flags_58d550|=1;initialize_sprite_device_vertices(s.vertices_58d540);h.register_cleanup_49cb29(h.context);}
    auto& a=s.vertices_58d540;
    resize_sprite_device_vertices(a,input.count_00,h);
    for(std::int32_t i=0;i<input.count_00;++i){
        auto& out=a.data_0c[i];auto& in=*input.data_0c[i];
        out.x_00=rounded(attribute(in,0x18));out.y_04=rounded(attribute(in,0x20));out.z_08=rounded(attribute(in,0x10));
        out.rhw_0c=rounded(0.25L/static_cast<long double>(out.z_08));
        out.color_10=(integer(attribute(in,0x50))<<24)|(integer(attribute(in,0x38))<<16)|(integer(attribute(in,0x40))<<8)|integer(attribute(in,0x48));
        out.u_18=rounded(static_cast<long double>(s.u_offset_58d2a0)+static_cast<long double>(attribute(in,0x28)));
        out.v_1c=rounded(static_cast<long double>(s.v_offset_58d2a8)+static_cast<long double>(attribute(in,0x30)));
    }
    h.draw_primitive_70(h.context,device.device_58cdb8,6,0x1c4,a.data_0c,static_cast<std::uint32_t>(a.count_00),0);
    //46DEB0 is an empty function; no counter is updated after drawing.
}
}
