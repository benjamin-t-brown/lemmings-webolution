#include "app/BackdropScene.h"
#include "app/Camera.h"
#include "app/Raster.h"
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include <cstdlib>
#include <fstream>
#include <iostream>

int main(){
 int checks=0,fail=0;auto check=[&](bool ok,const char* s){++checks;if(!ok){++fail;std::cerr<<"FAIL "<<s<<"\n";}};
 auto libdir=std::getenv("WEBOLUTION_GAME_DIR");if(!libdir){std::cout<<"SKIP: WEBOLUTION_GAME_DIR unset\n";return 0;}
 auto lib=webo::AssetLibrary::open(std::filesystem::path(libdir)/"lemmings.box",std::filesystem::path(libdir)/"patch.box");check(bool(lib),"open retail archive");if(!lib)return 1;
 std::string level;for(auto& n:webo::levelFileNames(*lib))if(n.find("MM_CLIMB_002")!=std::string::npos){level=n;break;}check(!level.empty(),"find supported level");if(level.empty())return 1;
 auto bytes=lib->read(level);check(bool(bytes),"read supported level");if(!bytes)return 1;
 auto scene=webo::app::BackdropScene::build(*lib,*bytes,"MM_CLIMB_002");check(bool(scene),"parse guarded roots");if(!scene)return 1;
 auto unsupported=webo::app::BackdropScene::build(*lib,*bytes,"MM_DIG_000");check(bool(unsupported)&&unsupported->empty(),"unsupported level is clean empty scene");
 webo::app::CameraParams cp;cp.width=640;cp.height=480;cp.aspect=4.0/3.0;auto cam=webo::app::buildCamera(cp);webo::app::Raster raster(640,480);raster.clear(0,0,0);webo::app::BackdropStats stats;scene->render(raster,cam,*lib,stats);
 std::size_t nonblack=0;for(std::size_t i=0;i<raster.framebuffer().rgb.size();i+=3)if(raster.framebuffer().rgb[i]||raster.framebuffer().rgb[i+1]||raster.framebuffer().rgb[i+2])++nonblack;
 check(stats.roots==4,"four retained roots");check(stats.meshRecords==1177,"source records preserved");check(stats.nullMaterials==304,"known null materials counted");check(stats.nullMaterialTriangles>0,"known null records draw with source diffuse");check(stats.billboards==17,"all retained type-2 source nodes submitted");check(stats.predicateRejected>0,"kind5 source predicate rejects records");check(nonblack>0,"scene-on changes pixels");
 std::cout<<checks<<" checks, "<<fail<<" failures; records="<<stats.meshRecords<<" null="<<stats.nullMaterials<<" null-triangles="<<stats.nullMaterialTriangles<<" billboards="<<stats.billboards<<" changed="<<nonblack<<"\n";return fail?1:0;
}
