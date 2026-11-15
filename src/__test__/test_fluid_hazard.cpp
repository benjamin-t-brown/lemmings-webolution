// Real installed routes and source-built framebuffers; never an original-game run.
#include "app/AppEntry.h"
#include "app/MapScene.h"
#include "assets/Png.h"
#include "assets/Box.h"
#include "sim/RetailLevel.h"
#include "sim/World.h"
#include "save/SaveStore.h"

#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <cmath>

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool ok, const std::string& why) {
  if (!ok) { ++failures; std::cerr << "FAIL: " << why << '\n'; }
}
std::string bytes(const fs::path& p) {
  std::ifstream in(p, std::ios::binary);
  return {std::istreambuf_iterator<char>(in), {}};
}
struct SavedFile {
  fs::path path; bool existed; std::string data;
  explicit SavedFile(fs::path p):path(std::move(p)),existed(fs::exists(path)),data(bytes(path)){}
  bool unchanged() const { return existed == fs::exists(path) && data == bytes(path); }
};
// Receiver membership in original rows at the visible feet. Keep the ground
// cell as a separate trace column to distinguish the existing 0x40 path.
int contact(const sim::World& w, const sim::Lemming& l) {
  const double row = w.terrain().height()-1-l.y;
  for (std::size_t i=0; i<w.setup().gates.size(); ++i) {
    const auto& g=w.setup().gates[i];
    double dx=l.x-g.minX, width=g.maxX-g.minX;
    if(width<0) width+=w.terrain().width();
    if(dx<0) dx+=w.terrain().width();
    if(g.height>0 && dx>=0 && dx<width && row>=g.minY && row<=g.minY+g.height)
      return int(i);
  }
  return -1;
}
void natural(const AssetLibrary& lib, const fs::path& out, const std::string& id) {
  auto setup=sim::loadRetailLevel(lib,id); check(bool(setup),"load "+id); if(!setup)return;
  std::ofstream data(out/(id+"-level-data.txt"));
  data<<"level="<<id<<" width="<<setup->terrain.width()<<" height="<<setup->terrain.height()<<'\n';
  for(const auto& h:setup->entrances) {
    data<<"hatch "<<h.x<<','<<h.y<<" sheet="<<h.sheetType<<'\n';
    check(h.sheetType==(id=="MM_W_012"?1:0),"real hatch carries expected species");
  }
  for(std::size_t i=0;i<setup->gates.size();++i) {
    const auto& g=setup->gates[i];
    data<<"receiver "<<i<<" id="<<g.id<<" kind="<<g.kind<<" bounds="<<g.minX<<','<<g.minY<<','<<g.maxX<<','<<g.maxY
        <<" height="<<g.height<<" low="<<g.low<<" high="<<g.high<<'\n';
  }
  sim::World w(std::move(*setup));
  std::ofstream trace(out/(id+"-world.csv"));
  std::ofstream all(out/(id+"-world-all.csv"));
  std::ofstream supported(out/(id+"-supported-prefix.csv"));
  bool reachedDryReceiver=false;
  all<<std::setprecision(17)<<"ms,index,x,y,state,sheet,frame,budget,dead,saved\n";
  trace<<"ms,index,x,y,state,sheet,gate,kind,height,feet_original_y,probe_original_y,static_40,zone_water,dead,saved\n";
  bool touched=false,drowned=false,crossed=false; int lastX=-1;
  for(int n=0;n<3000;++n) {
    w.advanceElapsedMs(20);
    for(std::size_t i=0;i<w.lemmings().size();++i) {
      const auto& a=w.lemmings()[i];
      all<<w.levelClockMs()<<','<<i<<','<<a.x<<','<<a.y<<','<<sim::stateName(a.state)<<','<<a.sheetType<<','
         <<a.frame<<','<<a.stepBudgetSeconds<<','<<w.dead()<<','<<w.saved()<<'\n';
    }
    if(w.lemmings().empty())continue;
    const auto& l=w.lemmings()[0]; int g=contact(w,l);
    // U3 intentionally changes the later empty-pit route. Keep an exact
    // before/after control up to its first dry receiver, plus the unchanged
    // public app prefix, rather than calling dry falls immunity.
    if(id=="MM_W_012") {
      const double row=w.terrain().height()-1-l.y;
      for(const auto& receiver:w.setup().gates)
        if(receiver.height<=0 && l.x>=receiver.minX && l.x<receiver.maxX &&
           row>=receiver.minY && row<=receiver.minY+receiver.high)
          reachedDryReceiver=true;
      if(!reachedDryReceiver)
        for(std::size_t index=0;index<w.lemmings().size();++index) {
          const auto& actor=w.lemmings()[index];
          supported<<w.levelClockMs()<<','<<index<<','<<actor.x<<','<<actor.y<<','
            <<sim::stateName(actor.state)<<','<<actor.sheetType<<','<<actor.frame<<','
            <<std::setprecision(17)<<actor.stepBudgetSeconds<<','<<w.saved()<<','<<w.dead()<<'\n';
        }
    }
    if(l.x!=lastX||g>=0||l.state==sim::State::Drown||l.state==sim::State::Dead) {
      trace<<w.levelClockMs()<<",0,"<<l.x<<','<<l.y<<','<<sim::stateName(l.state)<<','<<l.sheetType<<','<<g<<','
        <<(g<0?-1:w.setup().gates[g].kind)<<','<<(g<0?0:w.setup().gates[g].height)<<','
        <<w.terrain().height()-1-l.y<<','<<w.terrain().height()-2-l.y<<','<<w.terrain().waterHazard(l.x,l.y+1)<<','
        <<w.terrain().zoneWater(l.x,l.y+1)<<','<<w.dead()<<','<<w.saved()<<'\n';
    }
    lastX=l.x;
    touched|=g>=0; drowned|=l.state==sim::State::Drown;
    crossed|=g>=0&&l.state==sim::State::Walker;
    if(w.finished())break;
  }
  check(touched,"natural hatch route contacts real receiver: "+id);
  if(id=="MM_BUILD_BLOCK_A") {
#ifdef FLUID_HAZARD_after
    check(drowned&&w.dead()>0,"ordinary lemming drowns and counts lost on Build and Block");
#else
    check(crossed&&!drowned,"task-entry ordinary lemming crosses water alive");
#endif
  } else if(id=="MM_W_012") {
    check(crossed&&!drowned,"installed water hatch survives water contact");
#ifdef FLUID_HAZARD_after
    check(reachedDryReceiver&&w.dead()>0,"U3: water immunity does not prevent dry-pit splats");
#else
    check(w.dead()==0,"historical baseline has permanent dry-pit support");
#endif
  }
  std::ofstream(out/(id+"-world-summary.txt"))<<"touched="<<touched<<"\ndrowned="<<drowned
    <<"\ncrossed_walking="<<crossed<<"\ndead="<<w.dead()<<"\nsaved="<<w.saved()<<'\n';
}
void application(const fs::path& game,const fs::path& out,const std::string& id) {
  auto camera=out/(id+"-camera.json");
  std::ofstream(camera)<<"{\"viewport\":[640,480],\"yaw_units\":"<<(id=="MM_BUILD_BLOCK_A"?176:128)
      <<",\"level_eye_height\":"<<(id=="MM_BUILD_BLOCK_A"?40:96)<<"}\n";
  std::vector<std::string> args{"fluid-hazard","--game-dir",game.string(),"--level",id,"--sim","--native",
    "--camera",camera.string(),"--frames","300","--no-audio"};
  std::vector<char*> raw;for(auto& a:args)raw.push_back(a.data());
  app::ApplicationRunOptions options;options.frameSeconds=0.1;
  options.saveStore=std::make_shared<save::SaveStore>(std::make_unique<save::FileSaveSink>(out/(id+"-SavedGame.dat"),game));
  std::ofstream trace(out/(id+"-app.csv"));
  std::ofstream prefix(out/(id+"-app-supported-prefix.csv"));
  bool dryReceiver=false;
  trace<<"frame,ms,x,y,state,sheet,gate,dead,saved,clip,temporal,screen_left,screen_top,screen_right,screen_bottom\n";
  bool first=false,firstDrown=false,animated=false;
  const auto observe=[&](const app::LevelFrameView& f) {
    if(f.world.lemmings().empty())return;
    const auto& l=f.world.lemmings()[0];const int g=contact(f.world,l);
    if(id=="MM_W_012") {
      const double row=f.world.terrain().height()-1-l.y;
      for(const auto& receiver:f.world.setup().gates)
        if(receiver.height<=0 && l.x>=receiver.minX && l.x<receiver.maxX &&
           row>=receiver.minY && row<=receiver.minY+receiver.high)
          dryReceiver=true;
      if(!dryReceiver)
        prefix<<f.frame<<','<<f.world.levelClockMs()<<','<<l.x<<','<<l.y<<','<<sim::stateName(l.state)<<','
          <<l.frame<<','<<std::setprecision(17)<<l.stepBudgetSeconds<<','<<f.world.saved()<<','<<f.world.dead()<<'\n';
    }
    const auto actor=std::find_if(f.actors.begin(),f.actors.end(),[](const auto& a){return a.index==0;});
    trace<<f.frame<<','<<f.world.levelClockMs()<<','<<l.x<<','<<l.y<<','<<sim::stateName(l.state)<<','
      <<l.sheetType<<','<<g<<','<<f.world.dead()<<','<<f.world.saved()<<','
      <<(actor==f.actors.end()?"absent":actor->clip)<<','<<(actor==f.actors.end()?-1:actor->temporalIndex)<<','
      <<(actor==f.actors.end()?0:actor->placement.left)<<','<<(actor==f.actors.end()?0:actor->placement.top)<<','
      <<(actor==f.actors.end()?0:actor->placement.right)<<','<<(actor==f.actors.end()?0:actor->placement.bottom)<<'\n';
    if(g>=0&&!first) {check(writePng(out/(id+"-first-contact.png"),f.image).has_value(),"retain contact frame");first=true;}
    if(l.state==sim::State::Drown&&actor!=f.actors.end()) {
      check(std::string(actor->clip)=="drown","production drown SPR clip");
      if(!firstDrown){check(writePng(out/(id+"-drown.png"),f.image).has_value(),"retain drown frame");firstDrown=true;}
      if(actor->temporalIndex>0) animated=true;
    }
    if(f.frame==130||f.frame==138||f.frame==160||f.frame==200)
      check(writePng(out/(id+"-frame-"+std::to_string(f.frame)+".png"),f.image).has_value(),"retain matched frame");
  };
  check(app::runApplication(int(raw.size()),raw.data(),{},observe,options)==0,"public application "+id);
  check(first,"public application contacts fluid "+id);
#ifdef FLUID_HAZARD_after
  if(id=="MM_BUILD_BLOCK_A")check(firstDrown&&animated,"public route draws advancing drown animation");
#endif
}

#ifdef FLUID_HAZARD_after
// Placed-contact fixtures on installed volumes supplement (and do not replace)
// the natural hatch routes above. No species is inferred from an entrance name.
void speciesAndHeight(const AssetLibrary& lib,const fs::path& out) {
  std::ofstream log(out/"contact-fixtures.csv");
  log<<"level,gate,kind,sheet,height,feet_row,initial_state,result,dead\n";
  for(const auto& id:{"MM_BUILD_BLOCK_A","M20_MT_1","H17_mg_1"}) {
    auto loaded=sim::loadRetailLevel(lib,id);check(bool(loaded),std::string("fixture load ")+id);if(!loaded)continue;
    const int kind=std::string(id)=="MM_BUILD_BLOCK_A"?0:std::string(id)=="M20_MT_1"?2:1;
    auto gate=std::find_if(loaded->gates.begin(),loaded->gates.end(),[&](const auto& g){return g.kind==kind&&g.height>0;});
    check(gate!=loaded->gates.end(),std::string("installed filled receiver ")+id);if(gate==loaded->gates.end())continue;
    const auto index=std::size_t(gate-loaded->gates.begin());
    if(kind==2)check(std::any_of(loaded->entrances.begin(),loaded->entrances.end(),[](auto h){return h.sheetType==2;}),"installed acid hatch has sheet 2");
    for(int sheet=0;sheet<3;++sheet)for(auto state:{sim::State::Walker,sim::State::Faller,sim::State::Builder,sim::State::Blocker}) {
      auto setup=*loaded;setup.totalLemmings=0;
      sim::World world(std::move(setup));sim::Lemming l;
      l.x=int(std::ceil(gate->minX))+1;
      const int row=int(std::floor(gate->minY+gate->height));
      l.y=world.terrain().height()-1-row;l.sheetType=sheet;l.state=state;
      world.place(l);world.advanceElapsedMs(0);
      const bool fatal=kind==1||(kind==0&&sheet!=1)||(kind==2&&sheet!=2);
      check((world.lemmings()[0].state==sim::State::Drown)==fatal,"installed receiver species/state matrix");
      if(fatal) {
        check(world.dead()==0,"drown begins before loss count");
        world.advanceElapsedMs(2200);
        check(world.dead()==1&&world.lemmings()[0].state==sim::State::Dead,"drown completes and counts exactly once");
        world.advanceElapsedMs(1000);check(world.dead()==1,"dead actor does not recount");
      }
      log<<id<<','<<index<<','<<kind<<','<<sheet<<','<<gate->height<<','<<row<<','<<sim::stateName(state)<<','
        <<sim::stateName(world.lemmings()[0].state)<<','<<world.dead()<<'\n';
    }
  }
  // Actual linked water receiver 5 starts drained on MM_W_012. Send the same
  // type-4/type-5 messages the lever dispatches, ramp with the existing clock,
  // then place at the same point. This also checks the inclusive mean boundary
  // and rejects a bridge/actor's feet one row above the fluid.
  for(int phase=0;phase<4;++phase)for(int above=0;above<2;++above) {
    auto setup=sim::loadRetailLevel(lib,"MM_W_012");if(!setup){check(false,"height fixture load");continue;}
    setup->totalLemmings=0;
    auto gate=std::find_if(setup->gates.begin(),setup->gates.end(),[](const auto& g){return g.id==5;});
    if(gate==setup->gates.end()){check(false,"receiver 5 exists");continue;}
    const auto index=std::size_t(gate-setup->gates.begin());
    if(phase==1||phase==2)sim::dispatchLeverMessages(*setup,{{5,4,0,0}});
    if(phase==3) { // Authored-high starting fixture, then the real drain message.
      gate->height=gate->high;gate->request=0;gate->done=0;
      sim::dispatchLeverMessages(*setup,{{5,5,0,0}});
    }
    sim::World world(std::move(*setup));
    world.advanceElapsedMs(phase==1?6000:phase>=2?16000:0);
    const auto& current=world.setup().gates[index];
    const int row=int(std::floor(current.minY+(phase==1?current.height:current.high)))+above;
    sim::Lemming l;l.x=int(current.minX)+1;l.y=world.terrain().height()-1-row;l.state=sim::State::Walker;
    world.place(l);world.advanceElapsedMs(0);
    const bool fatal=(phase==1||phase==2)&&above==0;
    check((world.lemmings()[0].state==sim::State::Drown)==fatal,"current rising/raised/drained height and bridge clearance");
    log<<"MM_W_012,"<<index<<",0,0,"<<current.height<<','<<row<<",phase"<<phase<<','
      <<sim::stateName(world.lemmings()[0].state)<<','<<world.dead()<<'\n';
  }
}
#endif
}
int main(int argc,char** argv) {
  if(argc!=2||!std::getenv("WEBOLUTION_GAME_DIR"))return 2;
  const fs::path game=std::getenv("WEBOLUTION_GAME_DIR"),out=argv[1];fs::create_directories(out);
  const auto local=save::localAppDataDirectory();if(!local)return 2;
  SavedFile installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  auto map=app::MapScene::build(*lib,{});if(!map)return 2;
  check(map->levelForNode(14)==std::optional<std::string>("MM_BUILD_BLOCK_A"),"map node 14 installed id");
  check(map->titleForNode(14)==std::optional<std::string>("Build and Block"),"installed title corrects reported word order");
  // Diagnose any changed recorded Blocker route without weakening that driver's
  // outcome. Points come from its retained production actor/exit traces.
  if(auto setup=sim::loadRetailLevel(*lib,"MM_BLOCK_001_A")) {
    std::ofstream data(out/"MM_BLOCK_001_A-receivers.txt");
    for(std::size_t i=0;i<setup->gates.size();++i) {
      const auto& g=setup->gates[i];
      data<<i<<" id="<<g.id<<" kind="<<g.kind<<" bounds="<<g.minX<<','<<g.minY<<','<<g.maxX<<','<<g.maxY
          <<" height="<<g.height<<'\n';
    }
    sim::World w(std::move(*setup));
    for(const auto& p:{std::pair{183,76},std::pair{80,45},std::pair{130,91}}) {
      sim::Lemming l;l.x=p.first;l.y=p.second;
      data<<"point "<<l.x<<','<<l.y<<" feet_original_y="<<w.terrain().height()-1-l.y
          <<" probe_original_y="<<w.terrain().height()-2-l.y<<" gate="<<contact(w,l)
          <<" static_40="<<w.terrain().waterHazard(l.x,l.y+1)<<'\n';
    }
  } else check(false,"Blocker route receiver measurement loads");
#ifdef FLUID_HAZARD_after
  speciesAndHeight(*lib,out);
#endif
  for(const auto& id:{"MM_BUILD_BLOCK_A","MM_W_012"}) {natural(*lib,out,id);application(game,out,id);}
  check(installed.unchanged()&&normal.unchanged(),"installed and normal save unchanged");
  std::ofstream(out/"summary.txt")<<"failures="<<failures<<"\ninstalled_save_unchanged="<<installed.unchanged()
    <<"\nnormal_save_unchanged="<<normal.unchanged()<<'\n';
  return failures?1:0;
}
