#include "app/SceneDispatch.h"
#include <algorithm>

namespace webo::app {
namespace {
using M=std::array<double,12>;
M compose(const M&a,const M&b){M o{};for(int i=0;i<3;++i){for(int j=0;j<3;++j)for(int k=0;k<3;++k)o[3*i+j]+=a[3*i+k]*b[3*k+j];o[9+i]=b[9+i];for(int k=0;k<3;++k)o[9+i]+=a[9+k]*b[3*k+i];}return o;}
double depthOf(const M& composed,const M& sourceRotation){return (composed[9]*sourceRotation[2]+composed[10]*sourceRotation[5])+composed[11]*sourceRotation[8];}
}
std::vector<std::size_t> binaryInsertionOrder(std::span<const double> keys){
 std::vector<std::size_t> idx;
 if(keys.empty())return idx;
 idx.push_back(0);
 for(std::size_t i=1;i<keys.size();++i){
  std::size_t lo=0,hi=idx.size()-1;
  while(hi-lo>1){const std::size_t mid=(hi+lo)>>1;if(keys[i]<keys[idx[mid]])lo=mid;else hi=mid;}
  std::size_t at;
  if(keys[i]<keys[idx[hi]])at=hi+1;
  else if(keys[i]>keys[idx[lo]])at=lo;
  else at=lo+1;
  idx.insert(idx.begin()+static_cast<std::ptrdiff_t>(at),i);
 }
 return idx;
}
void dispatchSecondaryChildren(const SceneNode& root,const M& parentCamera,const M& sourceRotation,
 const std::function<void(const SecondaryDispatchItem&)>& visitor,const SecondaryDispatchOptions& options){
 const M parent=compose(root.matrix,parentCamera);
 std::vector<double> keys;keys.reserve(root.children.size());
 for(const auto& child:root.children){const M composed=compose(child.matrix,parent);keys.push_back(depthOf(composed,sourceRotation)+child.headDoubles[1]);}
 // Depth-first, file order, immediately after the owner (0x46C6F6..0x46C737, 0x46B6CC). The
 // visibility test precedes everything else in the original draw routines, so a hidden node
 // skips its subtree too.
 const auto emit=[&](const auto& self,const SceneNode& n,const M& ownerComposed,std::size_t depth,std::size_t parentOffset,double key)->void{
  const M composed=compose(n.matrix,ownerComposed);
  if(options.hidden&&options.hidden(n))return;
  if(options.originalGates){
   if(n.type==2&&n.type2.visibility==0)return;
   if(n.type==1&&depthOf(composed,sourceRotation)+n.headDoubles[0]<=0.0)return;
  }
  visitor({n.sourceOffset,n.type,key,depth,parentOffset});
  for(const auto& c:n.children)self(self,c,composed,depth+1,n.sourceOffset,depthOf(compose(c.matrix,composed),sourceRotation)+c.headDoubles[1]);
 };
 for(const std::size_t i:binaryInsertionOrder(keys))emit(emit,root.children[i],parent,1,root.sourceOffset,keys[i]);
}
}
