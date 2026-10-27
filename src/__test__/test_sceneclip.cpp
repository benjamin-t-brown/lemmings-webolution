#include "app/SceneClip.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
using namespace webo::app;
namespace {
void ck(bool b,const char* s){if(!b){std::cerr<<"FAIL: "<<s<<'\n';std::exit(1);}}
SceneClipInputVertex v(double x,double y,double z,double a){return {x,y,z,{a,a,a,a,a,a}};}
bool close(double a,double b,double e=1e-11){return std::abs(a-b)<=e*std::max(1.0,std::abs(b));}
SceneClipInputs cfg(double bias,int left=-10000000){return {1,bias,2e-6,1,1,0,0,10000000,-10000000,10000000,left};}
}
int main(){
  // Fresh near clip precedes projection: generated (0,2,eps) projects to y=-1e6,
  // not a linear interpolation of already-projected corners. Closing edge first.
  auto near=clipScenePolygon({v(-2,0,-2,0),v(2,0,2,1),v(2,4,2,1)},cfg(0));
  ck(bool(near)&&near->size()==4,"near retains positive polygon");
  ck(close((*near)[0].screenX,0)&&close((*near)[0].screenY,-1e6),"near crossing projected after epsilon overwrite");
  ck(close((*near)[0].viewX,0)&&close((*near)[0].viewY,2)&&close((*near)[0].viewZ,2e-6),"closing-edge native XYZ");
  ck(close((*near)[1].screenX,0)&&close((*near)[1].screenY,0),"second near crossing/order");
  ck(close((*near)[0].attributes[0],0.5),"near attributes use t");
  // Positive sub-epsilon source depth is retained, not overwritten.
  auto tiny=clipScenePolygon({v(0,0,1e-10,1),v(1,0,1,1),v(0,1,1,1)},cfg(0));
  ck(bool(tiny)&&tiny->size()==3&&(*tiny)[0].viewZ==1e-10,"positive sub-epsilon survives");
  // Projected triangle (-2,0,2),(2,0,4),(2,4,4), clipped X>=0.
  auto screen=clipScenePolygon({v(-4,0,2,0),v(8,0,4,1),v(8,-16,4,1)},cfg(0,0));
  ck(bool(screen),"screen clip accepted"); int crosses=0; bool sawY2=false,sawY0=false;
  for(const auto& q:*screen)if(close(q.screenX,0)){
    ++crosses;
    if(close(q.screenY,2)){sawY2=true;ck(close(q.attributes[0],1.0/3),"other screen field uses t, not clipping coordinate");}
    if(close(q.screenY,0)){sawY0=true;for(double a:q.attributes)ck(close(a,1.0/3),"all attributes use screen p");ck(close(q.viewZ,8.0/3),"screen depth p");ck(close(q.biasedNativeZ,8.0/3),"biased native Z field");}
  }
  ck(crosses==2&&sawY2&&sawY0,"asymmetric screen crossings preserved");
  // Y-plane counterpart: screen points (0,-2),(0,2),(4,2), clipped Y>=0.
  SceneClipInputs ycfg{1,0,2e-6,1,1,0,0,100,0,100,-100};
  auto yclip=clipScenePolygon({v(0,4,2,0),v(0,-8,4,1),v(16,-8,4,1)},ycfg);
  ck(bool(yclip),"Y-axis screen clipping accepted");bool ycross=false;
  for(const auto&q:*yclip)if(close(q.screenY,0)&&close(q.screenX,2)){ycross=true;ck(close(q.attributes[0],1.0/3),"Y-plane other X uses t");}
  ck(ycross,"asymmetric Y-plane crossing preserves X");
  // Bias is consumed once to form stored depth and affects p.
  auto biased=clipScenePolygon({v(-4,0,2,0),v(8,0,4,1),v(8,-16,4,1)},cfg(0.25,0));
  ck(bool(biased),"biased clip accepted");bool biasFound=false;
  for(const auto&q:*biased)if(close(q.screenX,0)&&close(q.screenY,0)){biasFound=true;ck(close(q.attributes[0],7.0/22),"bias applied once before screen p");}
  ck(biasFound,"biased crossing found");
  // Denominator zero selects H (higher clipping coordinate), regardless of
  // traversal orientation; here bias yields depths +1 and -1.
  auto zero=clipScenePolygon({v(-4,0,2,0),v(8,0,4,1),v(8,-16,4,1)},cfg(3,0));
  ck(bool(zero),"zero-denominator clip accepted");bool zeroFound=false;
  for(const auto&q:*zero)if(close(q.screenX,0)&&close(q.screenY,0)){zeroFound=true;ck(q.attributes[0]==1&&q.biasedNativeZ==1,"p0 selects high-coordinate endpoint");}
  ck(zeroFound,"zero-denominator crossing found");
  auto reversed=clipScenePolygon({v(8,-16,4,1),v(8,0,4,1),v(-4,0,2,0)},cfg(3,0));
  ck(bool(reversed),"reverse traversal accepted");zeroFound=false;
  for(const auto&q:*reversed)if(close(q.screenX,0)&&close(q.screenY,0)){zeroFound=true;ck(q.attributes[0]==1,"zero p endpoint independent of traversal");}
  ck(zeroFound,"reverse zero-denominator crossing found");
  auto equal=clipScenePolygon({v(0,0,1,1),v(1,0,1,1),v(0,-1,1,1)},cfg(0,0));
  ck(bool(equal),"boundary equality accepted");
  auto bad=clipScenePolygon({v(1,1,1,1)},{0,0,2e-6,1,1,0,0,10,-10,10,-10});
  ck(!bad,"invalid scale rejected");
  std::cout<<"sceneclip preprojection cases passed\n";
}
