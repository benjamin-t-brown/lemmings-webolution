#include "app/SceneDispatch.h"
#include <array>
#include <cmath>
#include <iostream>

int main(){
  webo::SceneNode root;root.sourceOffset=0x91782;root.matrix[0]=root.matrix[4]=root.matrix[8]=1;
  for(int i=0;i<3;++i){webo::SceneNode n;n.sourceOffset=100+i;n.type=(i==1?2:1);n.matrix[0]=n.matrix[4]=n.matrix[8]=1;n.matrix[11]=i;n.headDoubles[1]=i*10;root.children.push_back(n);}
  const std::array<double,12> identity{1,0,0,0,1,0,0,0,1,0,0,0};
  constexpr std::size_t want[]={102,101,100};std::size_t count=0;bool ok=true;
  webo::app::dispatchSecondaryChildren(root,identity,identity,[&](const auto& item){if(count>=3||item.nodeSourceOffset!=want[count]||!std::isfinite(item.key))ok=false;++count;});
  if(count!=3||!ok){std::cerr<<"secondary dispatch fixture failed\n";return 1;}std::cout<<"secondary dispatch fixture passed\n";return 0;
}
