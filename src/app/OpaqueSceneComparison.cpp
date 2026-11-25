#include "app/OpaqueSceneComparison.h"
#include "app/ActorOcclusion.h"
#include "assets/Png.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <format>
#include <fstream>
#include <iomanip>
#include <limits>

namespace webo::app {

OpaqueDraw opaqueRasterDraw(const PreparedRasterTriangle& triangle, bool depthTest) {
  SceneRasterOptions material{
      SceneFilter::Linear, triangle.address, SceneAlphaFunction::Disabled, 0, false,
      SceneBlendFactor::One, SceneBlendFactor::One,
      depthTest ? SceneDepthFunction::Less : SceneDepthFunction::Always, true,
      SceneShade::Flat, SceneRgbOperation::Modulate, SceneAlphaOperation::SelectTexture, 0.5, 0.5};
  material.highlight = triangle.highlight;
  material.highlightTextureRgb = true;
  OpaqueDraw draw{{}, material, nullptr, &triangle, 0, 0, {}, {}};
  const auto& tint = triangle.diffuse;
  for (std::size_t i = 0; i < draw.vertices.size(); ++i) {
    const auto& p = triangle.vertices[i];
    draw.vertices[i] = {p[0], p[1], p[2], p[3], {tint[0], tint[1], tint[2], 1},
                        triangle.uv[i][0], triangle.uv[i][1]};
  }
  return draw;
}

namespace {

using ScenePass = LevelScenePass;
using DrawKind = BackdropScene::PreparedStage::DrawKind;

constexpr std::string_view passName(ScenePass pass) {
  switch (pass) {
    case ScenePass::Primary: return "primary";
    case ScenePass::LegacyUnattributed: return "legacy-unattributed";
    case ScenePass::Tube: return "tube";
    case ScenePass::GadgetEffects: return "gadget-effects";
    case ScenePass::Pipes: return "pipes";
    case ScenePass::Fluid: return "fluid";
    case ScenePass::Secondary: return "secondary";
    case ScenePass::CounterGates: return "counter-gates";
    case ScenePass::BlueSky: return "blue-sky";
    case ScenePass::ExplosionMarks: return "explosion-marks";
    default: return {};
  }
}

constexpr std::string_view drawKind(DrawKind kind) {
  switch (kind) {
    case DrawKind::Mesh: return "mesh";
    case DrawKind::Billboard: return "billboard";
    case DrawKind::Sprite: return "sprite";
    case DrawKind::Fluid: return "fluid";
  }
  return {};
}

constexpr double kInfinity = std::numeric_limits<double>::infinity();

struct Pass {
  struct Range {
    std::uint64_t source;
    int band, bank;
    std::size_t first, end;
  };
  ScenePass stage;
  std::vector<OpaqueDraw> draws;
  std::vector<Range> ranges;
};

Pass backdropPass(ScenePass kind, const BackdropScene::PreparedStage& stage) {
  Pass pass{kind, {}, {}};
  pass.draws.reserve(stage.draws.size());
  for (const auto& record : stage.ranges) {
    const auto from = pass.draws.size();
    if (pass.ranges.empty() || pass.ranges.back().source != record.node)
      pass.ranges.push_back({record.node, -1, -1, from, from});
    for (const auto& draw : std::span(stage.draws).subspan(record.first, record.end - record.first)) {
      const auto* sprite = record.kind == DrawKind::Sprite ?
          &stage.stats.spriteSubmissions[*draw.submission] : nullptr;
      pass.draws.push_back({draw.triangle.vertices, draw.triangle.material, draw.triangle.texture,
                            nullptr, draw.source, draw.ordinal, record.material, drawKind(record.kind), sprite});
    }
    pass.ranges.back().end = pass.draws.size();
  }
  return pass;
}

Pass counterGatePass(std::span<const PreparedSceneTriangle> draws) {
  Pass pass{ScenePass::CounterGates, {}, {}};
  pass.draws.reserve(draws.size());
  for (const auto& draw : draws) {
    const auto gate = draw.material.highlight.index;
    const auto first = pass.draws.size();
    if (pass.ranges.empty() || pass.ranges.back().source != gate)
      pass.ranges.push_back({gate, -1, -1, first, first});
    pass.draws.push_back({draw.vertices, draw.material, draw.texture, nullptr, gate,
                         first - pass.ranges.back().first, "counter-gate", "gate"});
    ++pass.ranges.back().end;
  }
  return pass;
}

Pass tubePass(const TubeScene::PreparedTube& tube) {
  Pass pass{ScenePass::Tube, {}, {}};
  pass.draws.reserve(tube.triangles.size());
  const auto append = [&](std::size_t first, std::size_t end, std::uint64_t source, int band,
                          int bank, std::size_t materialIndex) {
    if (first == end) return;
    pass.ranges.push_back({source, band, bank, first, end});
    for (std::size_t i = first; i < end; ++i) {
      auto& draw = pass.draws.emplace_back(opaqueRasterDraw(tube.triangles[i], tube.depthTest));
      draw.source = materialIndex;
      draw.ordinal = i - first;
      draw.materialName = "tube-tile";
      draw.kind = "tube";
    }
  };
  std::size_t next = 0;
  for (const auto& block : tube.blocks) {
    append(next, block.first, next, -1, -1, SIZE_MAX);
    append(block.first, block.end, block.bucket, block.band, static_cast<int>(block.bank), block.material);
    next = block.end;
  }
  append(next, tube.triangles.size(), next, -1, -1, SIZE_MAX);
  return pass;
}

Pass explosionMarkPass(std::span<const PreparedSceneTriangle> triangles) {
  Pass pass{ScenePass::ExplosionMarks, {}, {}};
  if (triangles.empty()) return pass;
  pass.ranges.push_back({0, -1, -1, 0, triangles.size()});
  pass.draws.reserve(triangles.size());
  for (const auto& t : triangles)
    pass.draws.push_back({t.vertices, t.material, t.texture, nullptr, 0, pass.draws.size(),
                         "~splat", "explosion-mark"});
  return pass;
}

struct DepthInterval {
  double minimum = kInfinity, maximum = -kInfinity;
  std::size_t outOfRange = 0;

  explicit DepthInterval(std::span<const Pass> passes) {
    for (const auto& pass : passes) {
      if (pass.stage == ScenePass::ExplosionMarks) continue; // Read-only, unquantized Z test.
      for (const auto& draw : pass.draws)
        for (const auto& v : draw.vertices) {
          if (!std::isfinite(v.screenZ)) {
            ++outOfRange;
            continue;
          }
          minimum = std::min(minimum, v.screenZ);
          maximum = std::max(maximum, v.screenZ);
        }
    }
    if (minimum > maximum) minimum = maximum = 0;
    // The headroom leaves room for interpolation rounding and for a zero-span scene.
    const auto margin = std::max(1e-4, (maximum - minimum) * 0.01);
    minimum -= margin;
    maximum += margin;
  }

  double extent() const { return maximum - minimum; }
  double decode(float value) const {
    if (value == 1.0f) return kInfinity;
    if (!(value >= 0 && value <= 0.5f)) return std::numeric_limits<double>::quiet_NaN();
    return minimum + 2.0 * value * extent();
  }
  double quantization() const {
    // A full ulp at the upper edge of [0, 0.5], including decode roundoff.
    return std::ldexp(extent(), -23) +
        4.0 * std::max(std::abs(minimum), std::abs(maximum)) * std::numeric_limits<double>::epsilon();
  }
};

class CpuComparison {
 public:
  CpuComparison(int width, int height) : raster(width, height), scene{width, height, {}},
      owners(std::size_t(width) * height), mask(owners.size(), 1), pipeDepth(owners.size(), kInfinity) {
    scene.rgba.resize(owners.size() * 4, 0);
    for (std::size_t i = 0; i < owners.size(); ++i) scene.rgba[i * 4 + 3] = 255;
  }

  std::expected<void, std::string> draw(const Pass& pass) {
    const bool usesRgbRaster = pass.stage == ScenePass::Tube || pass.stage == ScenePass::BlueSky;
    if (usesRgbRaster) copyToRaster();
    if (pass.stage == ScenePass::BlueSky) {
      fillBlueSky(raster);
      copyFromRaster();
      return {};
    }
    SceneRasterObserver observer;
    observer.mask = &mask;
    observer.accepted = [&](const SceneRasterWriteEvent& event) { owners[event.pixel] = nextOwner; };
    for (const auto& draw : pass.draws) {
      if (draw.tube) {
        const auto& t = *draw.tube;
        raster.setDepthEnabled(draw.material.depthFunction == SceneDepthFunction::Less);
        raster.triangle(t.vertices, t.uv, *t.texture, t.diffuse, t.actorOwner, t.address, t.highlight,
                        {owners, nextOwner});
      } else {
        auto material = draw.material;
        if (pass.stage == ScenePass::Pipes && !draw.sprite) material.depthOutput = pipeDepth;
        if (const auto result = rasterizeSceneTriangle(scene, raster.mutableDepthBuffer(),
                *draw.backdrop, draw.vertices, material, &observer); !result)
          return result;
      }
      ++nextOwner;
    }
    if (usesRgbRaster) copyFromRaster();
    return {};
  }

  Image image() const {
    Image image;
    image.width = scene.width;
    image.height = scene.height;
    image.rgba = scene.rgba;
    for (std::size_t i = 0; i < owners.size(); ++i) image.rgba[i * 4 + 3] = 255;
    return image;
  }

  void drawActors(const PreparedLevelScene& frame, std::span<const double> drowningDepth) {
    copyToRaster();
    TubeScene::drawActors(raster, frame.camera, frame.actors, drowningDepth, pipeDepth);
    copyFromRaster();
  }

  void drawParticles(const PreparedLevelScene& frame) {
    copyToRaster();
    TubeScene::drawParticles(raster, frame.camera, frame.particles, pipeDepth);
    copyFromRaster();
  }

  Raster raster;
  SceneRgbaFramebuffer scene;
  std::vector<std::uint32_t> owners;
  std::vector<std::uint8_t> mask;
  std::vector<double> pipeDepth;
  std::uint32_t nextOwner = 1;

 private:
  void copyToRaster() {
    auto& rgb = raster.mutableFramebuffer().rgb;
    for (std::size_t i = 0; i < owners.size(); ++i)
      std::copy_n(scene.rgba.data() + 4 * i, 3, rgb.data() + 3 * i);
  }

  void copyFromRaster() {
    const auto& rgb = raster.framebuffer().rgb;
    for (std::size_t i = 0; i < owners.size(); ++i)
      std::copy_n(rgb.data() + 3 * i, 3, scene.rgba.data() + 4 * i);
  }
};

struct Candidate {
  std::uint32_t id;
  double depth;
  bool inside, edge;
};

struct PixelDifference {
  int x, y;
  int error;
  bool coverage, passCoverage, depth;
  std::vector<Candidate> candidates;
};

double cross(double ax, double ay, double bx, double by) { return ax * by - ay * bx; }

bool nearEdge(const SceneRasterVertex& a, const SceneRasterVertex& b, double x, double y) {
  const auto dx = b.screenX - a.screenX, dy = b.screenY - a.screenY;
  const auto rx = x - a.screenX, ry = y - a.screenY;
  const double length = dx * dx + dy * dy;
  const double t = length > 0 ? std::clamp((rx * dx + ry * dy) / length, 0.0, 1.0) : 0.0;
  const auto ex = rx - t * dx, ey = ry - t * dy;
  return ex * ex + ey * ey <= 1.0;
}

void findCandidates(std::vector<PixelDifference>& pixels, std::span<const Pass> passes, int height) {
  if (pixels.empty()) return;
  std::vector<std::size_t> rows(height + 1, pixels.size());
  for (std::size_t end = pixels.size(), y = height; y > 0; --y) {
    while (end > 0 && pixels[end - 1].y == static_cast<int>(y - 1)) --end;
    rows[y - 1] = end;
  }
  std::uint32_t id = 0;
  for (const auto& pass : passes) for (const auto& draw : pass.draws) {
    ++id;
    const auto& [a, b, c] = draw.vertices;
    const auto area = cross(b.screenX - a.screenX, b.screenY - a.screenY,
                            c.screenX - a.screenX, c.screenY - a.screenY);
    if (std::abs(area) < 1e-12 || !std::isfinite(area)) continue;
    const auto ox = draw.material.sampleOffsetX, oy = draw.material.sampleOffsetY;
    const auto xmin = std::min({a.screenX, b.screenX, c.screenX}) - ox - 1;
    const auto xmax = std::max({a.screenX, b.screenX, c.screenX}) - ox + 1;
    const auto ymin = std::clamp(std::ceil(std::min({a.screenY, b.screenY, c.screenY}) - oy - 1),
                                 0.0, double(height));
    const auto ymax = std::clamp(std::floor(std::max({a.screenY, b.screenY, c.screenY}) - oy + 1),
                                 -1.0, double(height - 1));
    for (int y = static_cast<int>(ymin); y <= ymax; ++y) {
      const auto rowEnd = pixels.begin() + rows[y + 1];
      auto it = std::lower_bound(pixels.begin() + rows[y], rowEnd, xmin,
                                [](const auto& p, double x) { return p.x < x; });
      for (; it != rowEnd && it->x <= xmax; ++it) {
        const double sx = it->x + ox, sy = y + oy;
        const double w0 = cross(b.screenX - sx, b.screenY - sy, c.screenX - sx, c.screenY - sy) / area;
        const double w1 = cross(c.screenX - sx, c.screenY - sy, a.screenX - sx, a.screenY - sy) / area;
        const double w2 = 1.0 - w0 - w1;
        const bool covered = std::min({w0, w1, w2}) >= 0;
        const bool edge = nearEdge(a, b, sx, sy) || nearEdge(b, c, sx, sy) || nearEdge(c, a, sx, sy);
        if (covered || edge)
          it->candidates.push_back({id, w0 * a.screenZ + w1 * b.screenZ + w2 * c.screenZ, covered, edge});
      }
    }
  }
}

std::expected<void, std::string> savePass(const std::filesystem::path& path, const CpuComparison& cpu,
    const OpaqueGpuReadback& gpu, std::span<const Pass> passes, const DepthInterval& interval,
    std::ostream& meta) {
  auto reference = cpu.image();
  Image heatmap = reference;
  std::array<std::size_t, 256> histogram{};
  std::array<int, 3> maximum{};
  std::size_t cpuCoverage = 0, gpuCoverage = 0, coverageMismatch = 0;
  std::size_t cpuPassCoverage = 0, gpuPassCoverage = 0, passCoverageMismatch = 0;
  std::size_t depthMismatch = 0, ownerMismatch = 0, invalidDepth = 0;
  double worstDepth = 0;
  std::vector<PixelDifference> differences;
  const auto firstPassOwner = cpu.nextOwner - passes.back().draws.size();
  for (std::size_t i = 0; i < reference.pixelCount(); ++i) {
    int error = 0;
    for (std::size_t k = 0; k < 3; ++k) {
      const int channel = std::abs(int(reference.rgba[4 * i + k]) - int(gpu.colour.rgba[4 * i + k]));
      error = std::max(error, channel);
      maximum[k] = std::max(maximum[k], channel);
    }
    ++histogram[error];
    const auto cpuDepth = cpu.raster.depthBuffer()[i];
    const auto gpuDepth = interval.decode(gpu.depth[i]);
    const bool cpuFinite = std::isfinite(cpuDepth), gpuFinite = std::isfinite(gpuDepth);
    cpuCoverage += cpuFinite;
    gpuCoverage += gpuFinite;
    invalidDepth += gpu.depth[i] != 1.0f && !(gpu.depth[i] >= 0 && gpu.depth[i] <= 0.5f);
    const bool coverage = cpuFinite != gpuFinite;
    coverageMismatch += coverage;
    const bool cpuPass = cpu.owners[i] >= firstPassOwner, gpuPass = gpu.owners[i] >= firstPassOwner;
    cpuPassCoverage += cpuPass;
    gpuPassCoverage += gpuPass;
    const bool passCoverage = cpuPass != gpuPass;
    passCoverageMismatch += passCoverage;
    const auto depthError = cpuFinite && gpuFinite ? std::abs(cpuDepth - gpuDepth) : 0.0;
    worstDepth = std::max(worstDepth, depthError);
    const bool depth = depthError > interval.quantization();
    depthMismatch += depth;
    const bool owner = cpu.owners[i] != gpu.owners[i];
    ownerMismatch += owner;
    heatmap.rgba[4 * i] = coverage || passCoverage || error > 1 ? 255 : 0;
    heatmap.rgba[4 * i + 1] = error == 1 ? 192 : 0;
    heatmap.rgba[4 * i + 2] = depth || owner ? 255 : 0;
    if (coverage || depth || owner || error)
      differences.push_back({int(i % reference.width), int(i / reference.width),
                             error, coverage, passCoverage, depth, {}});
  }
  findCandidates(differences, passes, reference.height);

  std::size_t edges = 0, interiorOverOne = 0;
  const auto pass = passName(passes.back().stage);
  std::ofstream mismatch(path.string() + "-mismatches.tsv");
  mismatch << std::setprecision(17)
           << "pass\tx\ty\tedge\tmax_rgb_error\tcoverage_diff\tpass_coverage_diff\tdepth_over_bound\tcpu_owner\tgpu_owner"
              "\tcpu_z\tgpu_z\tcpu_rgb\tgpu_rgb\tcandidates_id:z:inside:edge\n";
  for (const auto& pixel : differences) {
    const bool edge = std::ranges::any_of(pixel.candidates, &Candidate::edge);
    edges += edge;
    interiorOverOne += !edge && pixel.error > 1;
    if (!edge && !pixel.coverage && !pixel.depth && pixel.error <= 1 &&
        cpu.owners[std::size_t(pixel.y) * reference.width + pixel.x] ==
        gpu.owners[std::size_t(pixel.y) * reference.width + pixel.x]) continue;
    const auto i = std::size_t(pixel.y) * reference.width + pixel.x;
    mismatch << pass << '\t' << pixel.x << '\t' << pixel.y << '\t' << edge << '\t' << pixel.error
             << '\t' << pixel.coverage << '\t' << pixel.passCoverage << '\t' << pixel.depth
             << '\t' << cpu.owners[i] << '\t' << gpu.owners[i]
             << '\t' << cpu.raster.depthBuffer()[i] << '\t' << interval.decode(gpu.depth[i]);
    for (const auto* image : std::array<const Image*, 2>{&reference, &gpu.colour}) {
      mismatch << '\t' << unsigned(image->rgba[4 * i]) << ',' << unsigned(image->rgba[4 * i + 1])
               << ',' << unsigned(image->rgba[4 * i + 2]);
    }
    mismatch << '\t';
    for (const auto& candidate : pixel.candidates)
      mismatch << candidate.id << ':' << candidate.depth << ':' << candidate.inside << ':' << candidate.edge << ' ';
    mismatch << '\n';
  }
  if (!mismatch) return std::unexpected("cannot write opaque mismatch list " + path.string());
  meta << pass << '\t' << passes.back().draws.size() << '\t'
       << reference.pixelCount() << '\t' << cpuCoverage << '\t' << gpuCoverage << '\t' << coverageMismatch
       << '\t' << cpuPassCoverage << '\t' << gpuPassCoverage << '\t' << passCoverageMismatch
       << '\t' << ownerMismatch << '\t' << depthMismatch << '\t' << worstDepth << '\t' << invalidDepth
       << '\t' << reference.pixelCount() - histogram[0] << '\t' << maximum[0] << ',' << maximum[1]
       << ',' << maximum[2] << '\t' << edges << '\t' << interiorOverOne << '\t' << gpu.milliseconds << '\t';
  for (std::size_t i = 0; i < histogram.size(); ++i)
    if (histogram[i] != 0) meta << i << '=' << histogram[i] << ' ';
  meta << '\n';
  if (auto result = writePng(path.string() + "-cpu.png", reference); !result) return result;
  if (auto result = writePng(path.string() + "-gpu.png", gpu.colour); !result) return result;
  return writePng(path.string() + "-heatmap.png", heatmap);
}

struct FluidCheckpoints {
  std::vector<double> cpuBefore, cpuAfter;
  std::vector<float> gpuBefore, gpuAfter;
  std::vector<double> cpuBeforePipes;
  std::vector<float> gpuBeforePipes;
};

std::expected<void, std::string> reportDepthSnapshots(const SceneDepthSnapshots& gpu,
    const FluidCheckpoints& scene, std::span<const double> pipes, const DepthInterval& interval,
    std::ostream& meta) {
  const auto retained = gpu.beforeFluid == scene.gpuBefore && gpu.afterFluid == scene.gpuAfter;
  meta << "pre_water_depth_retained\t" << (gpu.beforeFluid == scene.gpuBefore)
       << "\npost_water_depth_retained\t" << (gpu.afterFluid == scene.gpuAfter) << '\n';
  if (!retained) return std::unexpected("a fluid depth snapshot changed after its pass");

  meta << "depth_plane\tcpu_finite\tgpu_finite\tcoverage_diff\tover_depth_bound\tmax_world_z_error\n";
  const auto compare = [&](std::string_view name, std::span<const double> cpu, std::span<const float> gl) {
    std::size_t cpuFinite = 0, gpuFinite = 0, coverage = 0, overBound = 0;
    double maximum = 0;
    for (std::size_t i = 0; i < cpu.size(); ++i) {
      const auto z = interval.decode(gl[i]);
      const bool a = std::isfinite(cpu[i]), b = std::isfinite(z);
      cpuFinite += a;
      gpuFinite += b;
      coverage += a != b;
      if (a && b) maximum = std::max(maximum, std::abs(cpu[i] - z));
      overBound += std::isnan(z) || (a && b && std::abs(cpu[i] - z) > interval.quantization());
    }
    meta << name << '\t' << cpuFinite << '\t' << gpuFinite << '\t' << coverage
         << '\t' << overBound << '\t' << maximum << '\n';
  };
  compare("accepted-pipes", pipes, gpu.pipes);
  compare("pre-water", scene.cpuBefore, gpu.beforeFluid);
  compare("post-water", scene.cpuAfter, gpu.afterFluid);
  std::size_t cpuPipeOrderErrors = 0, gpuPipeOrderErrors = 0;
  std::size_t cpuFluidDepthChanges = 0, gpuFluidDepthChanges = 0;
  for (std::size_t i = 0; i < pipes.size(); ++i) {
    cpuPipeOrderErrors += std::isfinite(pipes[i]) &&
        (pipes[i] != scene.cpuBefore[i] || !(pipes[i] < scene.cpuBeforePipes[i]));
    gpuPipeOrderErrors += gpu.pipes[i] != 1.0f &&
        (gpu.pipes[i] != gpu.beforeFluid[i] || !(gpu.pipes[i] < scene.gpuBeforePipes[i]));
    cpuFluidDepthChanges += scene.cpuBefore[i] != scene.cpuAfter[i];
    gpuFluidDepthChanges += gpu.beforeFluid[i] != gpu.afterFluid[i];
  }
  meta << "pipe_depth_order_errors\t" << cpuPipeOrderErrors << '\t' << gpuPipeOrderErrors
       << "\nfluid_depth_changes\t" << cpuFluidDepthChanges << '\t' << gpuFluidDepthChanges << '\n';
  return {};
}

std::expected<void, std::string> reportSky(const OpaqueGpuReadback& before,
    const OpaqueGpuReadback& sky, std::size_t cpuFills, std::ostream& meta) {
  std::size_t gpuFills = 0;
  const bool savedDepth = before.depth == sky.depth && before.owners == sky.owners;
  bool correct = true;
  for (std::size_t i = 0; i < before.colour.pixelCount(); ++i) {
    const auto* from = before.colour.rgba.data() + 4 * i;
    const auto* to = sky.colour.rgba.data() + 4 * i;
    const bool fill = before.depth[i] == 1.0f && from[0] == 0 && from[1] == 0 && from[2] == 0;
    gpuFills += fill;
    correct &= std::equal(to, to + 3, fill ? kBlueSkyRgb.data() : from) && from[3] == to[3];
  }
  meta << "sky_filled\t" << cpuFills << '\t' << gpuFills << "\nsky_preserved_depth_and_owner\t"
       << savedDepth << "\nsky_exact_operation\t" << correct << '\n';
  if (!correct || !savedDepth) return std::unexpected("sky fill changed a protected scene sample");
  return {};
}

bool sameBounds(const HatchHoverTarget& a, const HatchHoverTarget& b) {
  return a.index == b.index && a.left == b.left && a.top == b.top &&
         a.right == b.right && a.bottom == b.bottom;
}

double comparisonPointDepth(const Raster& raster, std::size_t pixel) {
  const double depth = raster.actorDepth().depthAt(pixel);
  return std::isfinite(depth) ? depth : raster.depthBuffer()[pixel];
}

void writePoint(std::ostream& out, std::string_view probe, int x, int y, double cpuDepth,
                std::size_t cpuOwner, const GlPointQuery& gpu) {
  out << "point\t" << probe << '\t' << x << '\t' << y
      << '\t' << cpuDepth << '\t' << gpu.pixel.depth << '\t' << cpuOwner << '\t' << gpu.pixel.owner
      << '\t' << int(gpu.pixel.kind) << '\t' << gpu.milliseconds << '\n';
}

std::expected<void, std::string> saveActors(const std::string& path, const PreparedLevelScene& frame,
    const CpuComparison& cpu, const GlActorReadback& gpu, std::ostream& meta) {
  auto reference = cpu.image();
  Image heatmap = reference;
  const auto& cpuActor = cpu.raster.actorDepth();
  std::vector<std::array<std::size_t, 3>> counts(frame.actors.boxes.size());
  for (const auto& actor : frame.actors.draws)
    if (actor.owner >= counts.size()) counts.resize(actor.owner + 1);
  std::ofstream differences(path + "-mismatches.tsv", std::ios::binary);
  std::ofstream owners(path + "-owners.tsv", std::ios::binary);
  differences << std::setprecision(17)
      << "x\ty\tcpu_actor\tgl_actor\tgl_kind\tcpu_actor_z\tgl_actor_z\tcpu_scene_owner"
         "\tcpu_scene_z\tgl_scene_z\tcpu_pipe_z\tgl_pipe_z\tmax_rgb\n";
  std::size_t ownerDiff = 0, coverageDiff = 0, actorRgbDiff = 0, overOne = 0, sameOwnerDepthDiff = 0;
  std::size_t sceneOverOne = 0;
  std::size_t drowningPixels = 0, bubblePixels = 0, throughFluid = 0;
  double maxActorDepthError = 0;
  int maxActorRgb = 0;
  for (std::size_t p = 0; p < gpu.actor.size(); ++p) {
    const auto& gl = gpu.actor[p];
    drowningPixels += gl.kind == GlActorPixel::Kind::Drowning;
    bubblePixels += gl.kind == GlActorPixel::Kind::Bubble;
    throughFluid += (gl.kind == GlActorPixel::Kind::Drowning || gl.kind == GlActorPixel::Kind::Bubble) &&
        gl.depth >= gpu.sceneDepth[p];
    const auto owner = cpuActor.ownerAt(p);
    const bool hasCpu = owner != ActorDepth::noOwner, hasGpu = gl.owner != ActorDepth::noOwner;
    if (hasCpu) ++counts[owner][0];
    if (hasGpu) ++counts[gl.owner][gl.kind == GlActorPixel::Kind::Brick ? 2 : 1];
    int error = 0;
    for (std::size_t c = 0; c < 3; ++c)
      error = std::max(error, std::abs(int(reference.rgba[4*p+c]) - int(gpu.colour.rgba[4*p+c])));
    heatmap.rgba[4*p] = static_cast<uint8_t>(std::min(error * 12, 255));
    heatmap.rgba[4*p+1] = owner == gl.owner ? 0 : 220;
    heatmap.rgba[4*p+2] = 0;
    if (!hasCpu && !hasGpu) {
      sceneOverOne += error > 1;
      continue;
    }
    const double cpuZ = cpuActor.depthAt(p);
    coverageDiff += hasCpu != hasGpu;
    ownerDiff += owner != gl.owner;
    actorRgbDiff += error > 0;
    overOne += error > 1;
    maxActorRgb = std::max(maxActorRgb, error);
    const bool unequalDepth = hasCpu && hasGpu && owner == gl.owner && cpuZ != gl.depth;
    sameOwnerDepthDiff += unequalDepth;
    if (hasCpu && hasGpu && owner == gl.owner)
      maxActorDepthError = std::max(maxActorDepthError, std::abs(cpuZ - gl.depth));
    if (owner == gl.owner && !unequalDepth && error == 0) continue;
    differences << p % frame.camera.width << '\t' << p / frame.camera.width << '\t' << owner
                << '\t' << gl.owner << '\t' << int(gl.kind) << '\t' << cpuZ << '\t' << gl.depth
                << '\t' << cpu.owners[p] << '\t' << cpu.raster.depthBuffer()[p] << '\t' << gpu.sceneDepth[p]
                << '\t' << cpu.pipeDepth[p] << '\t' << gpu.pipeDepth[p] << '\t' << error << '\n';
  }
  owners << std::setprecision(17)
      << "owner\tcpu_pixels\tgl_body_pixels\tgl_brick_pixels\tattachment_triangles\tdrowning"
         "\tleft\ttop\tright\tbottom\tplane_num\tplane_const\tplane_dx\tplane_dy\n";
  for (const auto& actor : frame.actors.draws) {
    const auto& c = counts[actor.owner];
    const auto& p = actor.body.placement;
    owners << actor.owner << '\t' << c[0] << '\t' << c[1] << '\t' << c[2]
           << '\t' << actor.attachment.size() << '\t' << actor.drowning << '\t'
           << p.left << '\t' << p.top << '\t' << p.right << '\t' << p.bottom;
    for (double coeff : ActorOcclusionPlane(frame.camera, actor.body.worldPosition).coefficients())
      owners << '\t' << coeff;
    owners << '\n';
  }
  meta << "actor_coverage_diff\t" << coverageDiff << "\nactor_owner_diff\t" << ownerDiff
       << "\nactor_rgb_diff\t" << actorRgbDiff << "\nactor_over_one\t" << overOne
       << "\nactor_max_rgb\t" << maxActorRgb << "\nactor_same_owner_depth_diff\t" << sameOwnerDepthDiff
       << "\nactor_max_world_z_error\t" << maxActorDepthError << "\nactor_readback_ms\t" << gpu.milliseconds
       << "\nactor_background_over_one\t" << sceneOverOne
       << "\ngl_drowning_body_pixels\t" << drowningPixels << "\ngl_bubble_pixels\t" << bubblePixels
       << "\ngl_drowning_through_completed_depth\t" << throughFluid << '\n';
  if (!differences || !owners) return std::unexpected("cannot write actor comparison " + path);
  if (auto saved = writePng(path + "-cpu.png", reference); !saved) return saved;
  if (auto saved = writePng(path + "-gpu.png", gpu.colour); !saved) return saved;
  return writePng(path + "-heatmap.png", heatmap);
}

std::expected<void, std::string> reportDrowning(const std::string& path, const PreparedLevelScene& frame,
    const CpuComparison& cpu, const GlActorReadback& gpu, std::span<const double> drowningDepth,
    std::ostream& meta) {
  std::ofstream log(path, std::ios::binary);
  log << std::setprecision(17)
      << "owner\tx\ty\tbody_z\tcpu_scenery\tgl_scenery\tcpu_pipe\tgl_pipe"
         "\tcpu_completed\tgl_completed\tinside_terrain\tcpu_body\tgl_body\n";
  const auto& camera = frame.camera;
  Image sprite{camera.width, camera.height, AlphaMode::Embedded, {}};
  sprite.rgba.resize(sprite.pixelCount() * 4);
  std::size_t through = 0, scenery = 0, pipes = 0;
  for (const auto& actor : frame.actors.draws) {
    if (!actor.drowning) continue;
    const auto& body = actor.body;
    std::ranges::fill(sprite.rgba, 0);
    if (!drawActorSprite(sprite, body.frame, body.sheet(), body.placement, body.mirrored,
                         {255, 255, 255}, true, body.inverted)) continue;
    const ActorOcclusionPlane plane(camera, actor.drowningPosition);
    const auto& footprint = frame.actors.footprint;
    for (std::size_t p = 0; p < sprite.pixelCount(); ++p) {
      if (sprite.rgba[4*p+3] == 0) continue;
      const int x = int(p % camera.width), y = int(p / camera.width);
      const double z = plane.depthAt(x + 0.5, y + 0.5);
      const bool inside = footprint && actorTexelInsideTerrain(footprint->lattice, footprint->collision,
          plane.worldAt(camera, x + 0.5, y + 0.5), footprint->gateThickness);
      const auto cpuZ = cpu.raster.depthBuffer()[p], glZ = gpu.sceneDepth[p];
      const auto cpuScenery = drowningDepth[p], glScenery = gpu.drowningDepth[p];
      const auto cpuPipe = cpu.pipeDepth[p], glPipe = gpu.pipeDepth[p];
      const auto& cpuDepth = cpu.raster.actorDepth();
      const bool cpuBody = cpuDepth.ownerAt(p) == actor.owner && cpuDepth.depthAt(p) == z;
      const bool glBody = gpu.actor[p].owner == actor.owner && gpu.actor[p].kind == GlActorPixel::Kind::Drowning;
      through += cpuBody && glBody && z >= cpuZ && z >= glZ && z < cpuScenery && z < glScenery;
      scenery += !inside && !cpuBody && !glBody && z >= cpuScenery && z >= glScenery &&
                 z < cpuPipe && z < glPipe;
      pipes += !inside && !cpuBody && !glBody && z < cpuScenery && z < glScenery &&
               z >= cpuPipe && z >= glPipe;
      log << actor.owner << '\t' << x << '\t' << y << '\t' << z << '\t' << cpuScenery << '\t' << glScenery
          << '\t' << cpuPipe << '\t' << glPipe << '\t' << cpuZ << '\t' << glZ << '\t' << inside
          << '\t' << cpuBody << '\t' << glBody << '\n';
    }
  }
  meta << "matched_drowning_body_through_fluid\t" << through
       << "\nmatched_drowning_body_blocked_by_scenery\t" << scenery
       << "\nmatched_drowning_body_blocked_by_pipe\t" << pipes << '\n';
  if (!log) return std::unexpected("cannot write drowning comparison " + path);
  return {};
}

struct EffectImages { Image cpu, gpu; };

int pixelDelta(const Image& before, const Image& after, std::size_t pixel, std::size_t channel) {
  return int(after.rgba[4 * pixel + channel]) - int(before.rgba[4 * pixel + channel]);
}

void reportEffectDelta(std::string_view name, const EffectImages& before, const EffectImages& after,
                       std::ostream& meta) {
  std::array<std::size_t, 511> histogram{};
  std::size_t cpuCount = 0, glCount = 0, changedMaskDiff = 0;
  int maximum = 0;
  for (std::size_t p = 0; p < after.cpu.pixelCount(); ++p) {
    bool cpuChanged = false, glChanged = false;
    int error = 0;
    for (std::size_t channel = 0; channel < 3; ++channel) {
      const auto cpu = pixelDelta(before.cpu, after.cpu, p, channel);
      const auto gl = pixelDelta(before.gpu, after.gpu, p, channel);
      cpuChanged |= cpu != 0;
      glChanged |= gl != 0;
      error = std::max(error, std::abs(cpu - gl));
    }
    cpuCount += cpuChanged;
    glCount += glChanged;
    changedMaskDiff += cpuChanged != glChanged;
    maximum = std::max(maximum, error);
    ++histogram[error];
  }
  meta << "effect_delta\t" << name << '\t' << cpuCount << '\t' << glCount << '\t' << changedMaskDiff
       << '\t' << maximum << '\t';
  for (std::size_t i = 0; i < histogram.size(); ++i)
    if (histogram[i] != 0) meta << i << '=' << histogram[i] << ' ';
  meta << '\n';
}

void reportMarkOverlap(const EffectImages& before, const EffectImages& after,
    const PreparedLevelScene& frame, const ActorDepth& cpu, const GlActorReadback& gl,
    std::ostream& meta) {
  std::vector<bool> drowning(frame.actors.boxes.size());
  for (const auto& actor : frame.actors.draws) {
    if (actor.owner >= drowning.size()) drowning.resize(actor.owner + 1);
    drowning[actor.owner] = actor.drowning;
  }
  std::size_t cpuCovered = 0, glCovered = 0;
  for (std::size_t p = 0; p < after.cpu.pixelCount(); ++p) {
    const auto owner = cpu.ownerAt(p);
    const auto opaqueGl = gl.actor[p].kind == GlActorPixel::Kind::Body ||
                          gl.actor[p].kind == GlActorPixel::Kind::Brick;
    bool cpuMark = false, glMark = false;
    for (std::size_t channel = 0; channel < 3; ++channel) {
      cpuMark |= pixelDelta(before.cpu, after.cpu, p, channel) != 0;
      glMark |= pixelDelta(before.gpu, after.gpu, p, channel) != 0;
    }
    cpuCovered += cpuMark && owner != ActorDepth::noOwner && !drowning[owner];
    glCovered += glMark && opaqueGl;
  }
  meta << "mark_pixels_replaced_by_opaque_actors\t" << cpuCovered << '\t' << glCovered << '\n';
}

std::expected<void, std::string> saveParticles(const std::string& path, const CpuComparison& cpu,
    const GlActorReadback& gpu, const GlActorReadback& before, const EffectImages& images,
    std::span<const double> beforeSceneDepth, std::ostream& meta) {
  auto heatmap = images.cpu;
  std::ofstream log(path + "-mismatches.tsv", std::ios::binary);
  log << std::setprecision(17)
      << "x\ty\tcpu_owner\tgl_owner\tcpu_z\tgl_z\tcpu_scene_z\tgl_scene_z\tprior_gl_owner\tprior_gl_z\tmax_rgb\n";
  std::size_t cpuCoverage = 0, glCoverage = 0, replacedGlActorPixels = 0;
  std::size_t ownerDiff = 0, overOne = 0;
  double maxDepthError = 0;
  const auto& actor = cpu.raster.actorDepth();
  for (std::size_t p = 0; p < gpu.actor.size(); ++p) {
    const auto owner = actor.ownerAt(p);
    const auto& gl = gpu.actor[p];
    const auto z = actor.depthAt(p);
    const bool cpuParticle = owner >= ActorDepth::firstParticleOwner && owner != ActorDepth::noOwner;
    const bool glParticle = gl.kind == GlActorPixel::Kind::Particle;
    cpuCoverage += cpuParticle;
    glCoverage += glParticle;
    replacedGlActorPixels += glParticle && before.actor[p].owner != ActorDepth::noOwner;
    int error = 0;
    for (std::size_t c = 0; c < 3; ++c)
      error = std::max(error, std::abs(int(images.cpu.rgba[4*p+c]) - int(images.gpu.rgba[4*p+c])));
    heatmap.rgba[4*p] = static_cast<std::uint8_t>(std::min(error * 12, 255));
    heatmap.rgba[4*p+1] = owner == gl.owner ? 0 : 220;
    heatmap.rgba[4*p+2] = 0;
    if (!cpuParticle && !glParticle) continue;
    ownerDiff += owner != gl.owner;
    overOne += error > 1;
    if (owner == gl.owner) maxDepthError = std::max(maxDepthError, std::abs(z - gl.depth));
    if (owner == gl.owner && z == gl.depth && error == 0) continue;
    log << p % images.cpu.width << '\t' << p / images.cpu.width << '\t' << owner << '\t' << gl.owner
        << '\t' << z << '\t' << gl.depth << '\t' << cpu.raster.depthBuffer()[p] << '\t' << gpu.sceneDepth[p]
        << '\t' << before.actor[p].owner << '\t' << before.actor[p].depth << '\t' << error << '\n';
  }
  meta << "particle_owner_pixels\t" << cpuCoverage << '\t' << glCoverage
       << "\nparticle_replaced_gl_actor_pixels\t" << replacedGlActorPixels
       << "\nparticle_owner_diff\t" << ownerDiff << "\nparticle_over_one\t" << overOne
       << "\nparticle_max_world_z_error\t" << maxDepthError << "\nparticle_readback_ms\t" << gpu.milliseconds
       << "\nparticle_preserved_scene_z\t" << std::ranges::equal(beforeSceneDepth, cpu.raster.depthBuffer())
       << '\t' << (before.sceneDepth == gpu.sceneDepth) << '\n';
  if (!log) return std::unexpected("cannot write particle comparison " + path);
  if (auto saved = writePng(path + "-cpu.png", images.cpu); !saved) return saved;
  if (auto saved = writePng(path + "-gpu.png", images.gpu); !saved) return saved;
  return writePng(path + "-heatmap.png", heatmap);
}

int blendSelector(const SceneRasterOptions& material) {
  if (!material.blendEnabled) return 0;
  if (material.destinationBlend == SceneBlendFactor::InvSrcAlpha) return 1;
  return material.sourceBlend == SceneBlendFactor::One ? 2 : 3;
}

}  // namespace

std::expected<void, std::string> OpaqueSceneComparison::draw(const PreparedLevelScene& frame,
    std::string_view level, int frameIndex, std::uint64_t levelClockMs, std::size_t terrainCellsChanged,
    std::size_t terrainBlocksRebuilt, bool checkpoint, const DisplayedFrameQueries* softwareQueries,
    int pointerX, int pointerY) {
  displayed_ = false;
  if (!frame.tube) return std::unexpected("scene comparison requires a prepared tube pass");
  if (std::ranges::find(frame.passes, LevelScenePass::Tube) == frame.passes.end())
    return std::unexpected("scene comparison requires the ordinary tube/actor pass order");
  std::vector<Pass> passes;
  passes.reserve(frame.passes.size());
  const auto append = [&](ScenePass pass, const BackdropScene::PreparedStage& stage) {
    passes.push_back(backdropPass(pass, stage));
    return passes.back().draws.size() == stage.draws.size();
  };
  for (const auto pass : frame.passes) {
    bool complete = true;
    switch (pass) {
      case ScenePass::Primary:
        if (frame.primary) complete = append(pass, *frame.primary);
        else passes.push_back({pass, {}, {}});
        break;
      case ScenePass::LegacyUnattributed: complete = append(pass, frame.legacy); break;
      case ScenePass::Tube: passes.push_back(tubePass(*frame.tube)); break;
      case ScenePass::GadgetEffects: complete = append(pass, frame.gadgetEffects); break;
      case ScenePass::Pipes: complete = append(pass, frame.pipes); break;
      case ScenePass::Fluid: complete = append(pass, frame.fluid); break;
      case ScenePass::Secondary: complete = append(pass, frame.secondary); break;
      case ScenePass::CounterGates: passes.push_back(counterGatePass(frame.counterGates)); break;
      case ScenePass::BlueSky: passes.push_back({pass, {}, {}}); break;
      case ScenePass::ExplosionMarks: passes.push_back(explosionMarkPass(frame.explosionMarks)); break;
      default: break; // Later phases consume the completed scene and its depth snapshots.
    }
    if (!complete) return std::unexpected(std::format("incomplete prepared ranges for {}", passName(pass)));
  }
  const DepthInterval interval(passes);
  const auto& camera = frame.camera;
  struct RestoreGl {
    OpaqueGpu& gpu;
    ~RestoreGl() { gpu.end(); }
  } restore{*gpu_};
  if (auto result = gpu_->begin(camera.width, camera.height, interval.minimum, interval.maximum,
                                frame.trackHighlights); !result)
    return result;

  std::filesystem::path base;
  std::ofstream meta, sources, uploaded;
  std::optional<CpuComparison> cpu;
  if (checkpoint) {
    std::error_code ec;
    if (levelDirectory_.empty()) {
      std::filesystem::create_directories(directory_, ec);
      if (ec) return std::unexpected(ec.message());
      for (int visit = 1; levelDirectory_.empty(); ++visit) {
        auto path = directory_ / std::format("{}-{:03}", level, visit);
        if (std::filesystem::create_directory(path, ec)) levelDirectory_ = std::move(path);
        if (ec) return std::unexpected(ec.message());
      }
    }
    base = levelDirectory_ / std::format("frame-{:06}", frameIndex);
    meta.open(base.string() + "-summary.tsv");
    sources.open(base.string() + "-materials.tsv");
    uploaded.open(base.string() + "-uploads.tsv");
    if (!meta || !sources || !uploaded) return std::unexpected("cannot open comparison files " + base.string());
    meta << std::setprecision(17) << "level\t" << level << "\nframe\t" << frameIndex
         << "\nlevel_clock_ms\t" << levelClockMs << "\nviewport\t" << camera.width << 'x' << camera.height
         << "\nselector_turns\t" << camera.selectorTurns << "\nworld_to_view\t";
    for (double value : camera.worldToView) meta << value << ' ';
    meta << "\nlens_center_focal\t" << camera.cx << ' ' << camera.cy << ' ' << camera.focalX << ' ' << camera.focalY
         << "\nbackend\tsoftware / native OpenGL\ndriver\t" << gpu_->driver()
         << "\nfragment_precision\t" << gpu_->precision()
         << "\nblend_arithmetic\tbyte-domain shader with a copied destination"
         << "\nterrain_cells_changed\t" << terrainCellsChanged
         << "\nterrain_blocks_rebuilt\t" << terrainBlocksRebuilt << "\ndepth_min\t" << interval.minimum
         << "\ndepth_max\t" << interval.maximum << "\ndepth_mapped\t[0,0.5]\ndepth_clear\t1 (infinity)"
         << "\ndepth_vertex_out_of_range\t" << interval.outOfRange << "\ndepth_error_bound\t"
         << interval.quantization() << '\n';
    meta << "pass\tdraws\tpixels\tcpu_coverage\tgpu_coverage\tcoverage_diff"
            "\tcpu_pass_coverage\tgpu_pass_coverage\tpass_coverage_diff"
            "\towner_diff\tdepth_over_bound\tmax_world_z_error\tdepth_pixel_out_of_range"
            "\tcolour_diff\tmax_rgb\tedge_diff\tinterior_over_one\treadback_ms\tmax_channel_error_histogram\n";
    sources << "id\tpass\tnode_or_bucket\tband\tbank\tsource_or_tile\ttriangle"
               "\tmaterial\tfilter\taddress\tshade\talpha_ref\tblend\tsrc_factor\tdst_factor\tdepth_func\tdepth_write"
               "\tkind\talpha_op\tblend_selector\tsprite_temporal\tmirror\n";
    uploaded << "pass\tnode_or_bucket\tband\tbank\ttriangles\tvertex_bytes\tindex_bytes\n";
    cpu.emplace(camera.width, camera.height);
  }

  FluidCheckpoints fluid;
  std::optional<OpaqueGpuReadback> beforeSky;
  std::optional<EffectImages> beforeMarks, afterMarks;
  std::vector<double> markCpuDepth;
  std::vector<float> markGlDepth;
  std::size_t cpuSkyFills = 0;
  std::uint32_t id = 1;
  for (std::size_t p = 0; p < passes.size(); ++p) {
    const auto& pass = passes[p];
    const auto name = passName(pass.stage);
    std::vector<OpaqueDrawRange> ranges;
    ranges.reserve(pass.ranges.size());
    for (const auto& range : pass.ranges) {
      const auto draws = std::span(pass.draws).subspan(range.first, range.end - range.first);
      ranges.push_back({range.source, range.band, range.bank, draws});
      if (checkpoint) for (const auto& draw : draws) {
        const auto& s = draw.material;
        sources << id++ << '\t' << name << '\t' << range.source << '\t' << range.band << '\t'
                << range.bank << '\t' << draw.source << '\t' << draw.ordinal << '\t'
                << (draw.materialName.empty() ? "(white)" : draw.materialName) << '\t' << int(s.filter)
                << '\t' << int(s.address) << '\t' << int(s.shade) << '\t'
                << (s.alphaFunction == SceneAlphaFunction::Disabled ? -1 : s.alphaReference)
                << '\t' << s.blendEnabled << '\t' << int(s.sourceBlend) << '\t' << int(s.destinationBlend)
                << '\t' << int(s.depthFunction) << '\t' << s.depthWrite << '\t' << draw.kind
                << '\t' << int(s.alphaOperation) << '\t' << blendSelector(s) << '\t';
        if (draw.sprite) sources << draw.sprite->temporal << '\t' << draw.sprite->mirror;
        else sources << '\t';
        sources << '\n';
      }
    }
    auto uploads = gpu_->draw(pass.stage, ranges);
    if (!uploads) return std::unexpected(uploads.error());
    if (!checkpoint) continue;
    for (const auto& upload : *uploads)
      uploaded << name << '\t' << upload.source << '\t' << upload.band << '\t' << upload.bank
               << '\t' << upload.triangles << '\t' << upload.vertexBytes << '\t' << upload.indexBytes << '\n';
    auto result = gpu_->read();
    if (!result) return std::unexpected(result.error());
    if (auto drawn = cpu->draw(pass); !drawn) return drawn;
    const auto path = base.string() + '-' + std::string(name);
    if (auto saved = savePass(path, *cpu, *result, std::span(passes).first(p + 1), interval, meta); !saved)
      return saved;
    switch (pass.stage) {
      case ScenePass::GadgetEffects:
        fluid.cpuBeforePipes = cpu->raster.depthBuffer();
        fluid.gpuBeforePipes = std::move(result->depth);
        break;
      case ScenePass::Pipes:
        fluid.cpuBefore = cpu->raster.depthBuffer();
        fluid.gpuBefore = std::move(result->depth);
        break;
      case ScenePass::Fluid:
        fluid.cpuAfter = cpu->raster.depthBuffer();
        fluid.gpuAfter = std::move(result->depth);
        break;
      case ScenePass::CounterGates:
        if (p + 1 < passes.size() && passes[p + 1].stage == ScenePass::BlueSky) {
          beforeSky = std::move(*result);
          const auto& depth = cpu->raster.depthBuffer();
          const auto& colour = cpu->scene.rgba;
          for (std::size_t i = 0; i < depth.size(); ++i)
            cpuSkyFills += depth[i] == kInfinity && colour[4*i] == 0 &&
                colour[4*i+1] == 0 && colour[4*i+2] == 0;
        }
        break;
      case ScenePass::BlueSky:
        if (auto checked = reportSky(*beforeSky, *result, cpuSkyFills, meta); !checked) return checked;
        break;
      case ScenePass::ExplosionMarks:
        if (beforeMarks) {
          afterMarks.emplace(EffectImages{cpu->image(), std::move(result->colour)});
          meta << "mark_preserved_scene_z\t" << (markCpuDepth == cpu->raster.depthBuffer())
               << '\t' << (markGlDepth == result->depth) << '\n';
          reportEffectDelta("explosion-marks", *beforeMarks, *afterMarks, meta);
        }
        break;
      default: break;
    }
    if (p + 1 < passes.size() && passes[p + 1].stage == ScenePass::ExplosionMarks &&
        !passes[p + 1].draws.empty()) {
      beforeMarks.emplace(EffectImages{cpu->image(), std::move(result->colour)});
      markCpuDepth = cpu->raster.depthBuffer();
      markGlDepth = std::move(result->depth);
    }
  }
  const bool hasActors = frame.actors.physical &&
      std::ranges::find(frame.passes, ScenePass::Actors) != frame.passes.end();
  const TubeScene::PreparedActors emptyActors;
  const bool patchFluid = std::ranges::find(frame.passes, ScenePass::CompleteDrowningDepth) != frame.passes.end();
  if (auto drawn = gpu_->drawActors(camera, hasActors ? frame.actors : emptyActors, patchFluid); !drawn)
    return drawn;
  std::optional<GlActorReadback> actorCheckpoint;
  if (checkpoint) {
    auto snapshots = gpu_->readDepthSnapshots();
    if (!snapshots) return std::unexpected(snapshots.error());
    if (auto checked = reportDepthSnapshots(*snapshots, fluid, cpu->pipeDepth, interval, meta); !checked)
      return checked;
    meta << "fluid_receiver\theight\traw\tdraws\n";
    for (const auto& receiver : frame.fluid.stats.fluidSubmissions)
      meta << receiver.entityIndex << '\t' << receiver.height << '\t' << receiver.raw
           << '\t' << receiver.rasterCalls << '\n';

    const auto drowningActors = std::ranges::count_if(frame.actors.draws, &TubeScene::PreparedActors::Actor::drowning);
    const bool drowningCheck = hasActors && patchFluid && drowningActors > 0;
    meta << "physical_actor_pass\t" << hasActors << "\ndrowning_actors\t" << drowningActors << '\n';
    if (hasActors) {
      if (patchFluid) patchDrowningDepth(fluid.cpuBefore, fluid.cpuAfter, cpu->raster.depthBuffer());
      cpu->drawActors(frame, patchFluid ? std::span<const double>(fluid.cpuBefore) : std::span<const double>());
    }
    auto actors = gpu_->readActors(drowningCheck);
    if (!actors) return std::unexpected(actors.error());
    if (auto saved = saveActors(base.string() + "-actors", frame, *cpu, *actors, meta); !saved)
      return saved;
    if (drowningCheck)
      if (auto checked = reportDrowning(base.string() + "-drowning-pixels.tsv", frame, *cpu,
              *actors, fluid.cpuBefore, meta); !checked)
        return checked;
    if (afterMarks) reportMarkOverlap(*beforeMarks, *afterMarks, frame, cpu->raster.actorDepth(), *actors, meta);
    actorCheckpoint.emplace(std::move(*actors));
  }
  const bool hasParticles = std::ranges::find(frame.passes, ScenePass::Particles) != frame.passes.end();
  if (hasParticles)
    if (auto drawn = gpu_->drawParticles(camera, frame.particles); !drawn) return drawn;
  if (checkpoint && hasParticles && !frame.particles.draws.empty()) {
    const auto cpuBefore = cpu->image();
    const auto beforeSceneDepth = cpu->raster.depthBuffer();
    cpu->drawParticles(frame);
    auto particles = gpu_->readActors();
    if (!particles) return std::unexpected(particles.error());
    const EffectImages images{cpu->image(), particles->colour};
    if (auto saved = saveParticles(base.string() + "-particles", *cpu, *particles, *actorCheckpoint,
            images, beforeSceneDepth, meta); !saved)
      return saved;
    reportEffectDelta("particles", EffectImages{cpuBefore, actorCheckpoint->colour}, images, meta);
    actorCheckpoint = std::move(*particles);
  }
  if (!frame.teleportEffects.draws.empty()) {
    const auto late = backdropPass(ScenePass::TeleportEffects, frame.teleportEffects);
    if (late.draws.size() != frame.teleportEffects.draws.size())
      return std::unexpected("incomplete prepared ranges for teleport effects");
    std::vector<OpaqueDrawRange> ranges;
    for (const auto& range : late.ranges)
      ranges.push_back({range.source, range.band, range.bank,
                        std::span(late.draws).subspan(range.first, range.end - range.first)});
    if (auto drawn = gpu_->draw(ScenePass::TeleportEffects, ranges); !drawn)
      return std::unexpected(drawn.error());
  }
  auto hatches = gpu_->readHatches(frame.hatchQueries);
  if (!hatches) return std::unexpected(hatches.error());
  hatches_ = std::move(hatches->targets);
  boxes_ = frame.actors.boxes;
  if (checkpoint) {
    const auto cpuHatches = softwareQueries
        ? std::vector<HatchHoverTarget>(softwareQueries->hatches().begin(), softwareQueries->hatches().end())
        : BackdropScene::hatchHoverTargets(camera, cpu->raster, frame.hatchQueries);
    std::ofstream queries(base.string() + "-queries.tsv", std::ios::binary);
    queries << std::setprecision(17);
    queries << "actor_boxes\t" << boxes_.size() << '\t'
            << (!softwareQueries || std::ranges::equal(boxes_, softwareQueries->actorBoxes())) << '\n';
    queries << "hatch_bounds\t" << cpuHatches.size() << '\t' << hatches_.size() << '\t'
            << std::ranges::equal(cpuHatches, hatches_, sameBounds) << '\t' << hatches->readbackMilliseconds << '\n';
    for (const auto& h : cpuHatches)
      queries << "hatch\tCPU\t" << h.index << '\t' << h.left << '\t' << h.top << '\t' << h.right << '\t' << h.bottom << '\n';
    for (const auto& h : hatches_)
      queries << "hatch\tGL\t" << h.index << '\t' << h.left << '\t' << h.top << '\t' << h.right << '\t' << h.bottom << '\n';
    queries << "point_fields\tprobe\tx\ty\tcpu_z\tgl_z\tcpu_actor\tgl_actor\tgl_kind\treadback_ms\n";
    if (pointerOverSceneBounds(pointerX, pointerY, 0, 0, camera.width, camera.height)) {
      auto pointer = gpu_->readPoint(pointerX, pointerY);
      if (!pointer) return std::unexpected(pointer.error());
      const std::size_t p = std::size_t(pointerY) * camera.width + pointerX;
      const auto depth = softwareQueries ? softwareQueries->pointDepth(pointerX, pointerY) : std::nullopt;
      const auto owner = softwareQueries ? softwareQueries->actorOwnerAt(pointerX, pointerY) :
          cpu->raster.actorDepth().ownerAt(p);
      writePoint(queries, "pointer", pointerX, pointerY, depth.value_or(comparisonPointDepth(cpu->raster, p)),
                 owner, *pointer);
      const auto hatchIndex = [&](auto targets) {
        const auto hit = hoveredHatch(targets, pointerX, pointerY);
        return hit ? targets[*hit].index : ActorDepth::noOwner;
      };
      queries << "pointer_hatch\t" << hatchIndex(std::span(cpuHatches)) << '\t' << hatchIndex(std::span(hatches_)) << '\n';
    }
    constexpr std::array names{"scene", "body", "brick", "drowning", "bubble", "particle"};
    for (std::size_t k = 0; k < names.size(); ++k) {
      const auto found = std::ranges::find(actorCheckpoint->actor, static_cast<GlActorPixel::Kind>(k),
                                          &GlActorPixel::kind);
      if (found == actorCheckpoint->actor.end()) continue;
      const auto p = std::size_t(found - actorCheckpoint->actor.begin());
      const int x = int(p % camera.width), y = int(p / camera.width);
      const auto sampled = gpu_->readPoint(x, y);
      if (!sampled) return std::unexpected(sampled.error());
      const auto depth = softwareQueries ? softwareQueries->pointDepth(x, y) : std::nullopt;
      const auto owner = softwareQueries ? softwareQueries->actorOwnerAt(x, y) : cpu->raster.actorDepth().ownerAt(p);
      writePoint(queries, names[k], x, y, depth.value_or(comparisonPointDepth(cpu->raster, p)), owner, *sampled);
    }
    if (!queries) return std::unexpected("cannot write displayed-frame comparison " + base.string());
  }
  if (checkpoint && (!meta || !sources || !uploaded))
    return std::unexpected("cannot write comparison files " + base.string());
  displayed_ = true;
  return {};
}

}  // namespace webo::app
