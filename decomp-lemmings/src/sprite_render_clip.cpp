#include "lemmings/sprite_render_clip.hpp"
#include <bit>
#include <cmath>
#include <cstring>
namespace lemmings {
namespace {
double get(const SpriteRenderVertex& v,unsigned i){double d;std::memcpy(&d,reinterpret_cast<const std::byte*>(&v)+8*i,8);return d;}
void put(SpriteRenderVertex& v,unsigned i,double d){std::memcpy(reinterpret_cast<std::byte*>(&v)+8*i,&d,8);}
double rounded(long double d){volatile double out=static_cast<double>(d);return out;}
std::int32_t add(std::int32_t a,std::int32_t b){return std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(a)+static_cast<std::uint32_t>(b));}
}
void interpolate_sprite_clip_vertex(SpriteRenderVertex& out,double plane,const SpriteRenderVertex& first,const SpriteRenderVertex& second,bool x_axis){
    const unsigned axis=x_axis?3:4,other=x_axis?4:3;
    const SpriteRenderVertex* high=&second;const SpriteRenderVertex* low=&first;
    if(get(first,axis)>get(second,axis)){high=&first;low=&second;}
    const long double hi=get(*high,axis),lo=get(*low,axis),p=plane;
    const double linear=rounded((p-hi)/(lo-hi));
    put(out,other,rounded((static_cast<long double>(get(*low,other))-get(*high,other))*linear+get(*high,other)));
    // Raw integer assignment in the original preserves the plane's NaN bits.
    std::memcpy(reinterpret_cast<std::byte*>(&out)+axis*8,&plane,8);
    double weight=rounded((lo-p)*get(*low,2)+(p-hi)*get(*high,2));
    // TEST AH,40 treats unordered like equality, leaving NaN denominator intact.
    if(weight!=0&&!std::isnan(weight))weight=rounded(((p-hi)*get(*high,2))/weight);
    for(unsigned i:{0u,1u,2u,5u,6u,7u,8u,9u}){
        const double difference=rounded(static_cast<long double>(get(*low,i))-get(*high,i));
        const double scaled=rounded(static_cast<long double>(weight)*difference);
        put(out,i,rounded(static_cast<long double>(get(*high,i))+scaled));
    }
    put(out,10,rounded((static_cast<long double>(get(*low,10))-get(*high,10))*weight+get(*high,10)));
}
SpriteRenderVertex* take_sprite_clip_vertex(SpriteClipScratch& s){
    if(s.available_10.count_00==0){const auto index=s.used_20++;return s.vertices_0c+std::bit_cast<std::int32_t>(index);}
    auto* result=s.available_10.data_0c[s.available_10.count_00-1];--s.available_10.count_00;return result;
}
void append_sprite_clip_pointer(SpriteVertexPointerArray& a,SpriteRenderVertex* const& value,const SpriteClipHooks& h){
    if(a.capacity_04<=a.count_00){
        a.capacity_04=add(a.capacity_04,a.growth_08);
        auto* p=h.allocate_49c1a8(h.context,static_cast<std::uint32_t>(a.capacity_04)*4);
        for(std::int32_t i=0;i<a.count_00;++i)p[i]=a.data_0c[i];
        h.free_49c13d(h.context,a.data_0c);a.data_0c=p;
    }
    a.data_0c[a.count_00]=value;a.count_00=add(a.count_00,1);
}
void clip_sprite_plane(SpriteVertexPointerArray& input,SpriteVertexPointerArray& output,SpriteClipScratch& scratch,double plane,SpriteClipPlane kind,const SpriteClipHooks& h){
    auto* previous=input.data_0c+input.count_00-1;auto* current=input.data_0c;auto remaining=input.count_00;
    output.count_00=0;
    const bool x_axis=kind==SpriteClipPlane::maximum_x||kind==SpriteClipPlane::minimum_x;
    const bool minimum=kind==SpriteClipPlane::minimum_x||kind==SpriteClipPlane::minimum_y;
    const auto outside=[&](const SpriteRenderVertex& v){const double coordinate=x_axis?v.x_18:v.y_20;return minimum?!(coordinate>=plane):coordinate>plane;};
    do{
        if(outside(**current)){
            if(!outside(**previous)){auto* point=take_sprite_clip_vertex(scratch);interpolate_sprite_clip_vertex(*point,plane,**previous,**current,x_axis);append_sprite_clip_pointer(output,point,h);}
        }else{
            if(outside(**previous)){auto* point=take_sprite_clip_vertex(scratch);interpolate_sprite_clip_vertex(*point,plane,**previous,**current,x_axis);append_sprite_clip_pointer(output,point,h);}
            append_sprite_clip_pointer(output,*current,h);
        }
        previous=current;++current;--remaining;
    }while(remaining>0);
}
void clip_and_draw_sprite_polygon(SpriteVertexPointerArray& input,SpriteClipState& s,const SpriteClipHooks& h){
    clip_sprite_plane(input,s.temporary_58d558,s.scratch_58d578,s.maximum_y_58d288,SpriteClipPlane::maximum_y,h);
    if(s.temporary_58d558.count_00==0)return;
    clip_sprite_plane(s.temporary_58d558,input,s.scratch_58d578,s.minimum_y_58d28c,SpriteClipPlane::minimum_y,h);
    if(input.count_00==0)return;
    clip_sprite_plane(input,s.temporary_58d558,s.scratch_58d578,s.maximum_x_58d284,SpriteClipPlane::maximum_x,h);
    if(s.temporary_58d558.count_00==0)return;
    clip_sprite_plane(s.temporary_58d558,input,s.scratch_58d578,s.minimum_x_58d280,SpriteClipPlane::minimum_x,h);
    if(input.count_00==0)return;
    h.draw_46d460(h.context,input);
}
}
