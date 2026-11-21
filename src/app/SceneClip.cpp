#include "app/SceneClip.h"
#include <algorithm>
#include <cmath>

namespace webo::app { namespace {
constexpr double LIMIT=1e100;
bool bounded(double x){return std::isfinite(x)&&std::abs(x)<=LIMIT;}
SceneClipInputVertex mix(const SceneClipInputVertex& h,const SceneClipInputVertex& l,double t){
  SceneClipInputVertex o;
  o.viewX=h.viewX+(l.viewX-h.viewX)*t; o.viewY=h.viewY+(l.viewY-h.viewY)*t;
  o.viewZ=h.viewZ+(l.viewZ-h.viewZ)*t;
  for(std::size_t i=0;i<6;++i)o.attributes[i]=h.attributes[i]+(l.attributes[i]-h.attributes[i])*t;
  return o;
}
SceneClipVertex mix(const SceneClipVertex& h,const SceneClipVertex& l,double p){
  SceneClipVertex o;
  o.viewX=h.viewX+(l.viewX-h.viewX)*p; o.viewY=h.viewY+(l.viewY-h.viewY)*p;
  o.viewZ=h.viewZ+(l.viewZ-h.viewZ)*p; o.biasedNativeZ=h.biasedNativeZ+(l.biasedNativeZ-h.biasedNativeZ)*p;
  o.screenX=h.screenX+(l.screenX-h.screenX)*p; o.screenY=h.screenY+(l.screenY-h.screenY)*p;
  for(std::size_t i=0;i<6;++i)o.attributes[i]=h.attributes[i]+(l.attributes[i]-h.attributes[i])*p;
  return o;
}
bool finite(const SceneClipInputVertex& v){
  if(!bounded(v.viewX)||!bounded(v.viewY)||!bounded(v.viewZ))return false;
  return std::all_of(v.attributes.begin(),v.attributes.end(),bounded);
}
bool finite(const SceneClipVertex& v){
  if(!bounded(v.viewX)||!bounded(v.viewY)||!bounded(v.viewZ)||!bounded(v.screenX)||!bounded(v.screenY)||!bounded(v.biasedNativeZ))return false;
  return std::all_of(v.attributes.begin(),v.attributes.end(),bounded);
}
void nearClip(std::vector<SceneClipInputVertex>& p,double scale,double epsilon){
  if(p.empty())return;
  std::vector<SceneClipInputVertex> out;out.reserve(p.size()+2);
  auto prev=p.back(); bool prevIn=scale*prev.viewZ>0;
  for(const auto& cur:p){
    const bool curIn=scale*cur.viewZ>0;
    if(prevIn!=curIn){
      const SceneClipInputVertex* high=&prev; const SceneClipInputVertex* low=&cur;
      if(high->viewZ<low->viewZ)std::swap(high,low);
      const double t=-(scale*high->viewZ)/(scale*low->viewZ-scale*high->viewZ);
      auto q=mix(*high,*low,t); q.viewZ=epsilon/scale; out.push_back(q);
    }
    if(curIn)out.push_back(cur);
    prev=cur;prevIn=curIn;
  }
  p=std::move(out);
}
template<bool XAxis,bool KeepLow>
void screenPlane(std::vector<SceneClipVertex>& p,double bound){
  if(p.empty())return;
  auto c=[](const SceneClipVertex& v){return XAxis?v.screenX:v.screenY;};
  auto inside=[&](double v){return KeepLow?v<=bound:v>=bound;};
  std::vector<SceneClipVertex> out;out.reserve(p.size()+2);
  auto prev=p.back();bool prevIn=inside(c(prev));
  for(const auto& cur:p){
    const bool curIn=inside(c(cur));
    if(prevIn!=curIn){
      const SceneClipVertex *h=&prev,*l=&cur;
      if(c(*h)<c(*l))std::swap(h,l); // H is higher clipping coordinate.
      const double hc=c(*h),lc=c(*l);
      const double t=(bound-hc)/(lc-hc);
      const double den=(bound-hc)*h->biasedNativeZ+(lc-bound)*l->biasedNativeZ;
      const double pweight=den==0?0:((bound-hc)*h->biasedNativeZ)/den;
      auto q=mix(*h,*l,pweight);
      if constexpr(XAxis){q.screenX=bound;q.screenY=h->screenY+(l->screenY-h->screenY)*t;}
      else {q.screenY=bound;q.screenX=h->screenX+(l->screenX-h->screenX)*t;}
      out.push_back(q);
    }
    if(curIn)out.push_back(cur);
    prev=cur;prevIn=curIn;
  }
  p=std::move(out);
}
}

std::expected<std::vector<SceneClipVertex>,std::string> clipScenePolygon(
 const std::vector<SceneClipInputVertex>& input,const SceneClipInputs& in){
  if(input.size()>4096||!bounded(in.nativeScale)||in.nativeScale<=0||
     !bounded(in.nativeBias)||!bounded(in.nearEpsilon)||in.nearEpsilon<=0||
     !bounded(in.projectionX)||!bounded(in.projectionY)||!bounded(in.centerX)||!bounded(in.centerY)||
     in.bottom<in.top||in.right<in.left)return std::unexpected("invalid clip inputs/domain");
  for(const auto& v:input)if(!finite(v))return std::unexpected("non-finite/out-of-domain source vertex");
  std::vector<SceneClipInputVertex> near=input;
  nearClip(near,in.nativeScale,in.nearEpsilon);
  std::vector<SceneClipVertex> p;p.reserve(near.size());
  for(const auto& v:near){
    if(!(v.viewZ>0))return std::unexpected("near result has nonpositive view depth");
    SceneClipVertex q;q.viewX=v.viewX;q.viewY=v.viewY;q.viewZ=v.viewZ;q.attributes=v.attributes;
    q.screenX=in.centerX+in.projectionX*(v.viewX/v.viewZ);
    q.screenY=in.centerY-in.projectionY*(v.viewY/v.viewZ);
    q.biasedNativeZ=in.nativeScale*v.viewZ-in.nativeBias;
    if(!finite(q))return std::unexpected("projection overflow/outside finite domain");
    p.push_back(q);
  }
  // Settled native order: bottom, top, right, left; equality is inside.
  screenPlane<false,true>(p,in.bottom);
  screenPlane<false,false>(p,in.top);
  screenPlane<true,true>(p,in.right);
  screenPlane<true,false>(p,in.left);
  for(const auto& v:p)if(!finite(v))return std::unexpected("screen clipping overflow/outside finite domain");
  return p;
}
} // namespace webo::app
