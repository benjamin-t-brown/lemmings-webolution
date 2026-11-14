// P8 clone-only evidence: installed natural hatch/ledge, a simulation-dug
// Basher tunnel, and a lit Bomber. Supplemental silhouette samples are labelled.
#include "app/AppEntry.h"
#include "app/RuntimeSkillPanel.h"
#include "app/ActorVisibility.h"
#include "app/LemmingAnim.h"
#include "app/TubeScene.h"
#ifdef OCCLUSION_HOVER_EXPECTATION
#include "app/BackdropScene.h"
#include "app/RuntimeCursor.h"
#include "app/SceneFrame.h"
#endif
#include "assets/Box.h"
#include "assets/LevelIndex.h"
#include "assets/Png.h"
#include "PanelIconComparison.h"
#include "sim/RetailLevel.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace webo;
namespace fs = std::filesystem;
namespace {
int failures = 0;
void check(bool ok, const std::string& message) {
  if (!ok) { ++failures; if (failures < 25) std::cerr << "FAIL: " << message << '\n'; }
}
std::string bytes(const fs::path& p) {
  std::ifstream in(p, std::ios::binary); return {std::istreambuf_iterator<char>(in), {}};
}
struct SaveSnapshot {
  fs::path path; bool exists; std::string data;
  explicit SaveSnapshot(fs::path p) : path(std::move(p)), exists(fs::exists(path)), data(bytes(path)) {}
  bool unchanged() const { return exists == fs::exists(path) && data == bytes(path); }
};
std::uint64_t hash(const app::Framebuffer& f) {
  std::uint64_t h = 14695981039346656037ull;
  for (auto c : f.rgb) { h ^= c; h *= 1099511628211ull; }
  return h;
}
Image image(const app::Framebuffer& fb) {
  Image im; im.width=fb.width; im.height=fb.height; im.rgba.resize(std::size_t(fb.width)*fb.height*4,255);
  for(std::size_t i=0;i<fb.rgb.size()/3;++i)
    std::copy_n(fb.rgb.begin()+i*3,3,im.rgba.begin()+i*4);
  return im;
}
std::size_t changed(const app::Framebuffer& a, const app::Framebuffer& b) {
  std::size_t n=0;
  for(std::size_t i=0;i<a.rgb.size();i+=3)
    n+=!std::equal(a.rgb.begin()+i,a.rgb.begin()+i+3,b.rgb.begin()+i);
  return n;
}
void png(const fs::path& path, const Image& im) { check(writePng(path,im).has_value(),"write "+path.string()); }
// Review aid only: nearest-neighbour enlargement of source-rendered pixels.
// Full-sized frames above remain the measurement artifacts.
Image enlargedCrop(const Image& im,int left,int top,int width,int height) {
  Image crop;crop.width=width*6;crop.height=height*6;
  crop.rgba.assign(std::size_t(crop.width)*crop.height*4,255);
  for(int y=0;y<crop.height;++y)for(int x=0;x<crop.width;++x) {
    const int sx=std::clamp(left+x/6,0,im.width-1),sy=std::clamp(top+y/6,0,im.height-1);
    std::copy_n(im.rgba.begin()+(std::size_t(sy)*im.width+sx)*4,4,
                crop.rgba.begin()+(std::size_t(y)*crop.width+x)*4);
  }
  return crop;
}
app::SceneOptions route() {
  app::SceneOptions o; o.drawCellRoute=true; o.drawWall=false; o.drawOuter=false; o.drawInner=false; return o;
}
app::Framebuffer render(const app::TubeScene& scene,const AssetLibrary& lib,
                        const app::CameraBasis& camera,const app::SceneOptions& options) {
  app::Raster r(camera.width,camera.height); r.clear(19,23,31); scene.render(r,camera,options,&lib); return r.framebuffer();
}
struct Record { std::uint64_t hash; int pixels, box; };
Record readRecord(const fs::path& p) { Record r{}; std::ifstream(p)>>r.hash>>r.pixels>>r.box; return r; }
void writeRecord(const fs::path& p,Record r) { std::ofstream(p)<<r.hash<<' '<<r.pixels<<' '<<r.box<<'\n'; }

int sceneCase(const AssetLibrary& lib,const std::string& level,const fs::path& out,
              const fs::path& baseline,bool before) {
  fs::create_directories(out);
  auto setup=sim::loadRetailLevel(lib,level); check(bool(setup),"load "+level); if(!setup)return 0;
  sim::World world(*setup);
  std::string levelPath;
  for(const auto& p:levelFileNames(lib))if(p.find(level)!=std::string::npos){levelPath=p;break;}
  auto blob=lib.read(levelPath); check(bool(blob),"read "+level);if(!blob)return 0;
  auto mesh=findCylinderMesh(*blob);auto index=indexLevel(*blob);
  check(bool(mesh)&&bool(index),"parse cylinder/index");if(!mesh||!index)return 0;
  auto scene=app::TubeScene::build(lib,*mesh,*blob,index->grid0);check(bool(scene),"build scene");if(!scene)return 0;
  app::LemmingAnimTracker anim;
  const auto step=[&] { world.advanceElapsedMs(100);anim.advance(world,*scene->lemmingSprites(),0.1); };
  bool bashed=false;int bashStart=0;
  if(level=="MM_BASH_002") {
    for(int n=0;n<1400;++n) {
      step();if(world.lemmings().empty())continue;
      const auto& l=world.lemmings()[0];
      // The accepted Basher route chooses the tall earth pillar, not the
      // thin steps or the steel seam. Start three cells ahead; the existing
      // grace cycles advance naturally into it.
      bool tall=true;
      for(int dy=0;dy<8;++dy) {
        const auto material=world.terrain().at(world.terrain().wrapX(l.x+3*int(l.facing)),l.y-dy);
        tall &= sim::isSolid(material)&&material!=sim::Material::Steel;
      }
      if(!bashed && l.state==sim::State::Walker && tall) {
        bashStart=l.x;bashed=world.assign(0,sim::Skill::Basher);
      }
      scene->syncTerrain(world.terrain());
      if(bashed && l.state==sim::State::Basher && std::abs(l.x-bashStart)>=5)break;
    }
    check(bashed&&scene->terrainCellsChanged()>0,"installed Basher excavates a real tunnel");
    check(!world.lemmings().empty()&&world.lemmings()[0].state==sim::State::Basher,"snapshot inside active tunnel");
  } else {
    for(int n=0;n<60;++n)step();
    if(level=="MM_BOMB")check(world.assign(0,sim::Skill::Bomber),"installed bomber has a live fuse");
    anim.advance(world,*scene->lemmingSprites(),0);
    scene->syncTerrain(world.terrain());
  }
  auto all=app::injectedWalkersFromWorld(world,world.terrain().height(),&anim);
  check(!all.empty(),"natural released lemmings");if(all.empty())return 0;
  std::ofstream(out/"state.txt")<<"level="<<level<<"\nlevel_ms="<<world.levelClockMs()
    <<"\nx="<<world.lemmings()[0].x<<"\ny="<<world.lemmings()[0].y
    <<"\nstate="<<sim::stateName(world.lemmings()[0].state)<<"\nterrain_cells_changed="<<scene->terrainCellsChanged()
    <<"\nfuse_ms="<<world.lemmings()[0].fuseMs<<'\n';
  std::ofstream trace(out/"rotation.csv");
  trace<<std::setprecision(17)<<"delta_yaw,yaw_degrees,far_facing,submission_box,visible_box,actor_pixels,baseline_pixels,frame_hash\n";
  int farRejected=0,nearVisible=0,baselineLeak=0,partial=0,compared=0;
  ActorPoseLevel poseLevel{int(scene->collisionGrid().height),index->grid0,mesh->surface,&*mesh};
  auto pose=actorPoseFromNativeSample(poseLevel,{all[0].plus8X,all[0].plus8Y,all[0].plus8Z},all[0].facing);
  // Fine samples around the perspective silhouette supplement the full 360-degree turn.
  std::vector<int> angles;for(int a=-128;a<=128;a+=4)angles.push_back(a);
  for(int a=55;a<=85;++a)angles.push_back(a);
  std::sort(angles.begin(),angles.end());angles.erase(std::unique(angles.begin(),angles.end()),angles.end());
  for(int delta:angles) {
    app::CameraParams params;params.yawUnits=all[0].plus8X+delta;
    const auto camera=app::buildCamera(params);
    const bool far=pose.modelPosition[0]*(camera.eyeWorld[0]-pose.modelPosition[0])+
                   pose.modelPosition[2]*(camera.eyeWorld[2]-pose.modelPosition[2])<0;
    auto options=route();const auto empty=render(*scene,lib,camera,options);
    options.injectedWalkers={all[0]};
    const auto boxes=scene->injectedWalkerScreenBoxes(camera,options,lib);
    const auto frame=render(*scene,lib,camera,options);
    const int pixels=int(changed(empty,frame));
    const auto recordPath=out/(std::to_string(delta)+".txt");
    const Record rec{hash(frame),pixels,int(bool(boxes[0]))};writeRecord(recordPath,rec);
    Record prior=rec;
    if(!baseline.empty()) {prior=readRecord(baseline/recordPath.filename());check(fs::exists(baseline/recordPath.filename()),"baseline record exists");++compared;}
    const bool submitted=app::actorSubmissionVisible(all[0].plus8X,all[0].plus8Z,true,
        app::tubePassBounds(camera.selectorTurns,scene->collisionGrid().cylinderExtent));
    if(far&&submitted) {
      baselineLeak+=prior.pixels>0;
      if(!boxes[0])++farRejected;
      if(!before && pixels>0&&pixels<prior.pixels)++partial;
      // This explicit diagnostic is how levelvisibility independently obtains
      // the hidden actor's projection. It must bypass P8 as well as native +29;
      // the production frame/boxes above must continue to apply both gates.
      auto diagnostic=options;diagnostic.composeSkipActorVisibility=true;
      const auto revealed=render(*scene,lib,camera,diagnostic);
      const auto revealedBoxes=scene->injectedWalkerScreenBoxes(camera,diagnostic,lib);
      check(bool(revealedBoxes[0])&&changed(empty,revealed)>0,
            "explicit visibility diagnostic reveals the far actor");
      if(!baseline.empty())check(hash(revealed)==prior.hash,
            "diagnostic far actor matches authentic unmasked baseline");
    }
    if(!far&&submitted) {
      ++nearVisible;
      if(before) {
        check(bool(boxes[0])&&pixels>0,"before: near-side overlay stays visible");
        if(!baseline.empty())check(rec.hash==prior.hash,"before: near-side frame identity");
      } else {
        // R3 correction: near-facing actors can be behind actual side walls.
        // The former blanket visibility/identity expectations encoded the human
        // playtest defect. Compare every pixel to the current scene Z instead.
        auto diagnostic=options;diagnostic.composeSkipActorVisibility=true;
        const auto raw=render(*scene,lib,camera,diagnostic);
        const auto rawBoxes=scene->injectedWalkerScreenBoxes(camera,diagnostic,lib);
        app::Raster geometry(camera.width,camera.height);geometry.clear(19,23,31);
        auto tube=options;tube.injectedWalkers.clear();scene->render(geometry,camera,tube,&lib);
        auto expected=empty;
        if(rawBoxes[0])for(std::size_t at=0;at<geometry.depthBuffer().size();++at)
          if(app::actorPhysicalDepth(*rawBoxes[0],camera.width)<geometry.depthBuffer()[at])
            std::copy_n(raw.rgb.begin()+at*3,3,expected.rgb.begin()+at*3);
        check(frame.rgb==expected.rgb,"near-facing pixels obey actual scene depth");
        check(bool(boxes[0])==(pixels>0),"near-facing picking follows visible pixels");
      }
    }
    if(!submitted && !baseline.empty())check(rec.hash==prior.hash,"no-actor frame pixel-identical");
    if(!before&&!boxes[0])check(pixels==0,"fully hidden actor changes no pixels");
    trace<<delta<<','<<params.yawUnits*360/256<<','<<far<<','<<submitted<<','<<bool(boxes[0])
      <<','<<pixels<<','<<prior.pixels<<','<<rec.hash<<'\n';
    // Retain matching before/after views. More detailed rotations remain in the CSV.
    if(delta%16==0 || delta==60 || delta==68 || delta==72 || (!before&&pixels>0&&pixels<prior.pixels)) {
      png(out/("yaw-"+std::to_string(delta)+".png"),image(frame));
      if(delta==0||delta==68)png(out/("tube-only-"+std::to_string(delta)+".png"),image(empty));
    }
  }
  check(nearVisible>20,"full rotation includes substantial near-side arc");
  if(before)check(baselineLeak>5,"before: actual submitted actor pixels behind cylinder");
  else {check(farRejected>5,"after: far-side actors removed from drawing and pick/countdown boxes");
    if(!baseline.empty())check(baselineLeak>5&&compared==int(angles.size()),"paired installed before measurements");}

  // Countdown uses the SAME boxes as picking and the app, including its original
  // placements for partially visible actors. A wholly hidden actor has no label.
  if(level=="MM_BOMB") {
    auto font=loadTexture(lib,"TEXTURES\\^Font.bmp");check(bool(font),"retail font");
    if(font)for(int delta:{0,68,80}) {
      app::CameraParams p;p.yawUnits=all[0].plus8X+delta;auto c=app::buildCamera(p);
      auto o=route();o.injectedWalkers={all[0]};auto boxes=scene->injectedWalkerScreenBoxes(c,o,lib);
      auto im=image(render(*scene,lib,c,o));const auto old=im.rgba;
      std::vector<app::LevelActorView> views;if(boxes[0])views.push_back({0,*boxes[0]});
      app::drawBomberCountdowns(im,*font,world,views);
      check(boxes[0] ? im.rgba!=old : im.rgba==old,"countdown follows actor visibility");
      if(!baseline.empty()&&delta==0) {
        const auto encoded=encodePng(im);
        check(std::string(encoded.begin(),encoded.end())==bytes(baseline/"countdown-0.png"),
              "near-side countdown frame unchanged");
      }
      png(out/("countdown-"+std::to_string(delta)+".png"),im);
    }
  }
  // Supplemental broad explosion frames on an installed pose exercise partial
  // silhouette clipping. This is an injected visual fixture, not a natural fuse
  // observation. Natural ledge/tunnel/fuse snapshots above are never substituted.
  if(level=="MM_CLIMB_002") {
    const auto* explode=scene->lemmingSprites()->find("explode");
    int clipped=0;
    if(explode)for(int temporal=0;temporal<int(explode->animations.size());++temporal) {
      auto actor=all[0];actor.clip="explode";actor.walkTemporalIndex=temporal;
      for(int delta:{56,60,64,68}) {
        app::CameraParams p;p.yawUnits=actor.plus8X+delta;auto c=app::buildCamera(p);
        auto o=route();const auto empty=render(*scene,lib,c,o);o.injectedWalkers={actor};
        const auto frame=render(*scene,lib,c,o);const int n=int(changed(empty,frame));
        const auto stem="supplement-explode-"+std::to_string(temporal)+"-"+std::to_string(delta);
        writeRecord(out/(stem+".txt"),{hash(frame),n,0});
        if(!baseline.empty()) {
          const auto old=readRecord(baseline/(stem+".txt"));
          if(n>0&&n<old.pixels) {++clipped;if(clipped<=2)png(out/(stem+".png"),image(frame));}
        }
        if(before && n>0)png(out/(stem+".png"),image(frame));
        if(temporal==9&&delta==60)png(out/(stem+"-enlarged.png"),enlargedCrop(image(frame),80,235,45,40));
      }
    }
    if(!before&&!baseline.empty())check(clipped>0,"broad sprite is partially cut at silhouette");
    std::ofstream(out/"supplement-summary.txt")<<"partially_clipped="<<clipped<<'\n';
  }
  std::ofstream(out/"summary.txt")<<"near_visible="<<nearVisible<<"\nfar_rejected="<<farRejected
    <<"\nbaseline_leak_views="<<baselineLeak<<"\npartial_views="<<partial<<"\npaired_views="<<compared<<'\n';
  return partial;
}

void push(SDL_Event e) { check(SDL_PushEvent(&e)==1,"queue owned-process SDL input"); }
void key(SDL_Scancode code) { SDL_Event e{};e.type=SDL_KEYDOWN;e.key.keysym.scancode=code;push(e); }
void motion(int x,int y,int dx=0) {
  SDL_Event e{};e.type=SDL_MOUSEMOTION;e.motion.x=x;e.motion.y=y;e.motion.xrel=dx;push(e);
}
void button(bool down,int x,int y,Uint8 which) {
  SDL_Event e{};e.type=down?SDL_MOUSEBUTTONDOWN:SDL_MOUSEBUTTONUP;
  e.button.button=which;e.button.x=x;e.button.y=y;push(e);
}
void click(int x,int y) {button(true,x,y,SDL_BUTTON_LEFT);button(false,x,y,SDL_BUTTON_LEFT);}
struct AppRecord {
  int frame=0;std::uint64_t ms=0;int x=-1,y=-1;bool paused=false;double yaw=0;bool box=false;
  double left=0,top=0,right=0,bottom=0;
};
int appCase(const fs::path& game,const fs::path& out,const fs::path& baseline,bool before) {
  fs::create_directories(out);std::ofstream trace(out/"frames.txt");
  const bool paired=!baseline.empty();std::ifstream prior(baseline/"frames.txt");
  if(paired)check(bool(prior),"paired application baseline exists");
  app::ApplicationRunOptions options;options.frameSeconds=0.1;
  options.saveStore=std::make_shared<save::SaveStore>(
      std::make_unique<save::FileSaveSink>(out/"SavedGame.dat",game));
  std::vector<std::string> args{"actor-occlusion-app","--level","MM_CLIMB_002","--sim",
    "--native","--frames","160","--game-dir",game.string()};
  std::vector<char*> raw;for(auto& a:args)raw.push_back(a.data());
  bool requestedPause=false,assigned=false,farChecked=false,nearChecked=false;
  int pausedFrames=0,stock=0,frames=0,noActorFrames=0;std::uint64_t frozenMs=0;
  // R5/R6 deliberately change these old hovered frames. Keep the authentic
  // baseline main/scene and all exact comparisons; add only the new feedback
  // to its pixels, leaving camera, simulation and ordinary rendering under test.
  const auto compareFrame = [&](const std::string& name, const app::LevelFrameView& f,
                                const std::string& message) {
    const auto rawName = name + ".rgba";
    if (before) {
      std::ofstream raw(out/rawName,std::ios::binary);
      raw.write(reinterpret_cast<const char*>(f.image.rgba.data()),f.image.rgba.size());
      check(bool(raw),"retain authentic baseline pixels");
    }
    if (!paired) return;
    check(f.selectedSkillIndex==0&&app::runtimeSkillPanelIndex(f.cursorX,f.cursorY)<0,
          "historical comparison has only selected Climber animation");
    auto comparisonLib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");
    check(bool(comparisonLib),"open panel expectation assets");if(!comparisonLib)return;
    const auto iconPixels=test::climberIconComparisonPixels(*comparisonLib);
    Image expectedPanel=f.image;
    const auto baselinePixels=bytes(baseline/rawName);
    check(baselinePixels.size()==expectedPanel.rgba.size(),"baseline raw frame exists");
    if(baselinePixels.size()!=expectedPanel.rgba.size())return;
    expectedPanel.rgba.assign(baselinePixels.begin(),baselinePixels.end());
    test::normalizeClimberIcon(expectedPanel,iconPixels);
    auto expectedEncoded=encodePng(expectedPanel);
    std::string expectedBytes(expectedEncoded.begin(),expectedEncoded.end());
#ifdef OCCLUSION_HOVER_EXPECTATION
    auto lib = AssetLibrary::open(game/"lemmings.box",game/"patch.box");
    check(bool(lib),"open expected-feedback assets"); if(!lib)return;
    auto font=loadTexture(*lib,"TEXTURES\\^FONT.BMP");
    auto cursor=app::RuntimeCursor::load(*lib);
    check(bool(font)&&bool(cursor),"load expected-feedback font/cursor");if(!font||!cursor)return;
    const auto hatch = !f.hoveredActor ? app::hoveredHatch(f.hatches,f.cursorX,f.cursorY) : std::nullopt;
    check(f.markedActor==f.hoveredActor,"feedback marks the existing selected actor only");
    check(f.hoveredHatch==(hatch?std::optional<std::size_t>{f.hatches[*hatch].index}:std::nullopt),
          "hatch feedback follows displayed hit bounds with actor precedence");
    if(f.hoveredActor||hatch) {
      Image expected=f.image;
      const auto raw=bytes(baseline/rawName);
      check(raw.size()==expected.rgba.size(),"authentic baseline raw frame exists");
      if(raw.size()!=expected.rgba.size())return;
      expected.rgba.assign(raw.begin(),raw.end());
      if(f.hoveredActor) {
        const auto displayed=std::find_if(f.actors.begin(),f.actors.end(),
            [&](const auto& a){return a.index==*f.hoveredActor;});
        check(displayed!=f.actors.end(),"selected actor has a displayed sprite");
        if(displayed==f.actors.end())return;
        auto level=sim::resolveRetailLevel(*lib,std::string(f.levelName));if(!level){check(false,"resolve expected level");return;}
        auto data=lib->read(level->file);if(!data){check(false,"read expected level");return;}
        auto mesh=findCylinderMesh(*data);auto index=indexLevel(*data);
        if(!mesh||!index){check(false,"parse expected level");return;}
        auto scene=app::TubeScene::build(*lib,*mesh,*data,index->grid0);
        auto backdrop=app::BackdropScene::build(*lib,*data,std::string(f.levelName));
        if(!scene||!backdrop){check(false,"build expected feedback depth");return;}
        auto effects=backdrop->entranceExitPresentation(*data);
        if(!effects){check(false,"read expected hatch pose");return;}
        backdrop->setEntranceExitPresentation(*effects,f.hatchOpeningProgress,0);
        const auto camera=app::buildCamera(f.displayedPose);
        app::Raster depth(camera.width,camera.height);depth.clear(0,0,0);
        app::BackdropStats stats;auto geometry=route();
        app::scheduleSceneFrame(
            [&]{backdrop->renderStage(depth,camera,*lib,stats,app::BackdropScene::Stage::Primary);},
            [&]{backdrop->renderStage(depth,camera,*lib,stats,app::BackdropScene::Stage::LegacyUnattributed);},
            [&]{scene->render(depth,camera,geometry,&*lib);},
            [&]{backdrop->renderStage(depth,camera,*lib,stats,app::BackdropScene::Stage::FluidGate);},
            [&]{backdrop->renderStage(depth,camera,*lib,stats,app::BackdropScene::Stage::Secondary);});
        const auto& l=f.world.lemmings()[*f.hoveredActor];
        app::SceneOptions::InjectedNativeWalker walker;
        walker.plus8X=displayed->drawnSample[0];walker.plus8Y=displayed->drawnSample[1];walker.plus8Z=displayed->drawnSample[2];
        walker.facing=int(l.facing);walker.walkTemporalIndex=displayed->temporalIndex;walker.clip=displayed->clip;
        geometry.injectedWalkers={walker};
        check(scene->drawHoveredWalker(expected,0,camera,geometry,*lib,depth),"compose expected selected-sprite feedback");
      } else app::drawHatchHover(expected,*font,f.hatches[*hatch]);
      // Restore the later cursor where feedback overlaps it. The route's pause
      // caption is already present below y=422, outside every feedback extent;
      // drawing its translucent background twice would change the old pixels.
      cursor->draw(expected,f.cursorState,f.cursorX,f.cursorY);
      test::normalizeClimberIcon(expected,iconPixels);
      const auto encoded=encodePng(expected);
      expectedBytes.assign(encoded.begin(),encoded.end());
      png(out/(name+"-expected-feedback.png"),expected);
    }
#endif
    auto actual=f.image;
    test::normalizeClimberIcon(actual,iconPixels);
    const auto encoded=encodePng(actual);
    check(std::string(encoded.begin(),encoded.end())==expectedBytes,message);
  };
  const auto observe=[&](const app::LevelFrameView& f) {
    ++frames;AppRecord r;r.frame=f.frame;r.ms=f.world.levelClockMs();r.paused=f.paused;r.yaw=f.displayedPose.yawUnits;
    if(!f.world.lemmings().empty()){r.x=f.world.lemmings()[0].x;r.y=f.world.lemmings()[0].y;}
    const auto a=std::find_if(f.actors.begin(),f.actors.end(),[](const auto& actor){return actor.index==0;});
    if(a!=f.actors.end()){r.box=true;r.left=a->placement.left;r.top=a->placement.top;r.right=a->placement.right;r.bottom=a->placement.bottom;}
    trace<<std::setprecision(17)<<r.frame<<' '<<r.ms<<' '<<r.x<<' '<<r.y<<' '<<r.paused<<' '<<r.yaw
      <<' '<<r.box<<' '<<r.left<<' '<<r.top<<' '<<r.right<<' '<<r.bottom<<'\n';
    AppRecord old=r;
    if(paired) {
      prior>>old.frame>>old.ms>>old.x>>old.y>>old.paused>>old.yaw>>old.box>>old.left>>old.top>>old.right>>old.bottom;
      check(bool(prior)&&r.frame==old.frame&&r.ms==old.ms&&r.x==old.x&&r.y==old.y&&r.paused==old.paused&&r.yaw==old.yaw,
            "application simulation/clock/camera identical to authentic before route");
      if(r.box&&old.box)check(r.left==old.left&&r.top==old.top&&r.right==old.right&&r.bottom==old.bottom,
                             "visible application actor retains original pick rectangle");
    }
    if(std::none_of(f.world.lemmings().begin(),f.world.lemmings().end(),
                    [](const auto& actor){return actor.alive();})) {
      ++noActorFrames;
      const auto name="no-actor-frame-"+std::to_string(f.frame);
      compareFrame(name,f,"full no-lemming frame equals authentic baseline plus hatch feedback");
      png(out/(name+".png"),f.image);
    }
    if(!requestedPause&&r.ms>=6000) {requestedPause=true;key(SDL_SCANCODE_P);}
    if(f.paused) {
      ++pausedFrames;const int q=pausedFrames;
      if(q==1){frozenMs=r.ms;stock=f.world.setup().skills[0];}
      check(r.ms==frozenMs,"camera inspection leaves paused simulation frozen");
      const int px=int((old.left+old.right)/2),py=int((old.top+old.bottom)/2);
      if(q==1)motion(px,py);
      if(q==2) {
        check(r.box&&f.hoveredActor==0,"ordinary near-side actor is pickable");nearChecked=true;
        compareFrame("near",f,"whole near-side frame equals authentic baseline plus selected-actor feedback");
        png(out/"near.png",f.image);button(true,320,240,SDL_BUTTON_RIGHT);
      }
      if(q==3)motion(320,240,-230);
      if(q==5){button(false,px,py,SDL_BUTTON_RIGHT);motion(px,py);}
      if(q==7) {
        if(before)check(r.box&&f.hoveredActor==0,"before: cylinder-hidden actor is drawn and pickable");
        else {
          check(!r.box&&f.hoveredActor!=0,"after: cylinder-hidden actor is absent and not pickable");
          if(paired)click(px,py);
        }
        farChecked=true;png(out/"far.png",f.image);
        if(before||paired)png(out/"far-enlarged.png",enlargedCrop(f.image,px-24,py-24,48,48));
      }
      if(q==9) {
        check(f.world.setup().skills[0]==stock,"click on hidden actor spends no skill");
        button(true,320,240,SDL_BUTTON_RIGHT);
      }
      if(q==10)motion(320,240,230);
      if(q==12){button(false,px,py,SDL_BUTTON_RIGHT);motion(px,py);}
      if(q==14) {
        check(r.box&&f.hoveredActor==0,"near-side actor still pickable after rotation");
        compareFrame("returned-near",f,"whole returned-near frame equals authentic baseline plus selected-actor feedback");
        png(out/"returned-near.png",f.image);click(px,py);
      }
    }
    if(pausedFrames>=14&&!f.paused) {
      check(f.world.lemmings()[0].canClimb&&f.world.setup().skills[0]==stock-1,
            "public near-side click assigns Climber and resumes");
      assigned=true;png(out/"assigned.png",f.image);SDL_Event e{};e.type=SDL_QUIT;push(e);
    }
  };
  check(app::runApplication(int(raw.size()),raw.data(),{},observe,options)==0,"public application exits");
  check(noActorFrames>0,"public application no-actor frames exercised");
  check(assigned&&nearChecked&&farChecked&&pausedFrames==14,"public near/far/return/assignment route completes");
  if(paired){std::string extra;std::getline(prior,extra);check(!std::getline(prior,extra),"paired application frame counts equal");}
  std::ofstream(out/"app-route-summary.txt")<<"frames="<<frames<<"\npaused_frames="<<pausedFrames
    <<"\nnear_pick_checked="<<nearChecked<<"\nfar_pick_checked="<<farChecked
    <<"\nreturned_near_assigned="<<assigned<<"\nno_actor_frames="<<noActorFrames<<"\npaired_simulation_camera="<<paired<<'\n';
  return failures?1:0;
}
}
int main(int argc,char** argv) {
  if(argc<3)return 2;
  const std::string mode=argv[1];const bool before=mode=="before"||mode=="app-before";const fs::path out=argv[2];
  const fs::path baseline=argc>3&&std::string(argv[3])!="-"?fs::path(argv[3]):fs::path{};
  const char* gameEnv=std::getenv("WEBOLUTION_GAME_DIR");if(!gameEnv)return 2;
  const fs::path game=gameEnv;auto local=save::localAppDataDirectory();if(!local)return 2;
  SaveSnapshot installed(game/"SavedGame.dat"),normal(save::cloneSavePathUnder(*local));
  auto lib=AssetLibrary::open(game/"lemmings.box",game/"patch.box");if(!lib)return 2;
  fs::create_directories(out);
  if(mode.starts_with("app-"))appCase(game,out,baseline,before);
  else for(const std::string level:{"MM_CLIMB_002","MM_BASH_002","MM_BOMB"})
      sceneCase(*lib,level,out/level,baseline.empty()?fs::path{}:baseline/level,before);
  check(installed.unchanged()&&normal.unchanged(),"installed and normal saves byte-identical");
  std::ofstream(out/"summary.txt")<<"before="<<before<<"\ninstalled_save_unchanged="<<installed.unchanged()
    <<"\nnormal_save_unchanged="<<normal.unchanged()<<"\nfailures="<<failures<<'\n';
  std::cout<<"actor-occlusion "<<(before?"before":"after")<<": "<<failures<<" failures; "<<out.string()<<'\n';
  return failures?1:0;
}
