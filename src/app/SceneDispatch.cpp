#include "app/SceneDispatch.h"
#include <algorithm>

namespace webo::app {
namespace {
using M=std::array<double,12>;
M compose(const M&a,const M&b){M o{};for(int i=0;i<3;++i){for(int j=0;j<3;++j)for(int k=0;k<3;++k)o[3*i+j]+=a[3*i+k]*b[3*k+j];o[9+i]=b[9+i];for(int k=0;k<3;++k)o[9+i]+=a[9+k]*b[3*k+i];}return o;}
}
void dispatchSecondaryChildren(const SceneNode& root,const M& parentCamera,const M& sourceRotation,
 const std::function<void(const SecondaryDispatchItem&)>& visitor){
 std::vector<SecondaryDispatchItem> items;const M parent=compose(root.matrix,parentCamera);
 for(const auto& child:root.children){const M composed=compose(child.matrix,parent);const double z=(composed[9]*sourceRotation[2]+composed[10]*sourceRotation[5])+composed[11]*sourceRotation[8];items.push_back({child.sourceOffset,child.type,z+child.headDoubles[1]});}
 // The retained finite keys are distinct; tie behavior is outside this seam's
 // certified domain (native insertion is intentionally not modeled here).
 std::sort(items.begin(),items.end(),[](const auto&a,const auto&b){return a.key>b.key;});
 for(const auto& item:items)visitor(item);
}
}
