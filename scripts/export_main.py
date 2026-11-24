"""Build and check the source tree for the public branch."""

from hashlib import sha256
from pathlib import Path
import re
import shutil


ROOT = Path(__file__).resolve().parents[1]
OVERLAY = ROOT / "release/main"
OUTPUT = ROOT / "build/main-export"

ROOT_FILES = {
    ".clang-format": "C++ formatting rules.",
    ".gitattributes": "LF line endings.",
    "CMakeLists.txt": "Configure and link the game.",
    "cmake/webolution_app.cmake": "Compile the application object library.",
    "cmake/webolution_sources.cmake": "Select the production translation units.",
    "src/app/icon/generate_ico.py": "Regenerate the clone's Windows icon.",
}

CLONE_ICON_HASHES = {
    "src/app/icon/webolution.png": "1e0aa7fe2ce790c480e959ec94604bd129fb0009a61c4169a6d0d73a6453f308",
    "src/app/icon/webolution.ico": "f830b17e6eb42e96b1b4b8973efb354633ea9d42c414705280e0479a72288d75",
}

# List production files individually so additions to the checkout need review.
SOURCE_GROUPS = {
    "app": ("Game entry, SDL presentation, input, or rendering.", """
        ActorAnim.cpp ActorAnim.h ActorDepth.h ActorOcclusion.h ActorPose.cpp ActorPose.h
        ActorSprite.cpp ActorSprite.h ActorTerrain.cpp ActorTerrain.h ActorVisibility.cpp
        ActorVisibility.h AppEntry.h BackdropScene.cpp BackdropScene.h BuilderBrick.h Camera.cpp
        Camera.h CameraInput.cpp CameraInput.h CameraIntro.cpp CameraIntro.h CrtRand.cpp
        CrtRand.h CursorPolicy.cpp CursorPolicy.h DeathParticles.h EnglishStrings.h
        EntryStartCue.cpp EntryStartCue.h ExitBalloon.h FrontendSession.cpp FrontendSession.h
        FrameConsumption.h FrameProfile.h FrontendWindow.cpp FrontendWindow.h GameDataLocation.cpp
        GameDataLocation.h HatchAnimation.cpp
        HatchAnimation.h HoverFeedback.h
        LemmingAnim.cpp LemmingAnim.h LemmingCountdown.cpp LemmingMotion.cpp LemmingMotion.h
        LevelEndTransition.cpp LevelEndTransition.h LevelMenu.cpp LevelMenu.h LevelOutcome.h
        LevelStartSequence.h MapScene.cpp MapScene.h Pick.cpp Pick.h Raster.cpp Raster.h
        RasterRows.h Replay.hpp ResultsScene.cpp ResultsScene.h RuntimeCursor.cpp RuntimeCursor.h
        RuntimeHud.cpp RuntimeHud.h RuntimePick.cpp RuntimePick.h RuntimeSkillPanel.cpp
        RuntimeSkillPanel.h SceneBillboardSource.cpp SceneBillboardSource.h SceneClip.cpp
        SceneClip.h SceneDepth.cpp SceneDepth.h SceneDiffuse.cpp SceneDiffuse.h
        SceneDispatch.cpp SceneDispatch.h SceneFrame.h SceneMaterialFlags.h SceneMeshSource.cpp
        SceneMeshSource.h SceneRaster.cpp SceneRaster.h SceneVertexPack.cpp SceneVertexPack.h
        SdlShutdown.hpp SdlWindow.hpp SimClock.cpp SimClock.h SkillPanel.cpp SkillPanel.h
        SoundMenu.cpp SoundMenu.h TouchInput.cpp TouchInput.h TubeScene.cpp TubeScene.h Version.rc.in
        WindowScale.h main.cpp AppIconResource.h WindowsWindowIcon.cpp WindowsWindowIcon.h webolution.rc
    """),
    "assets": ("Original data decoding or asset lookup.", """
        Bitmap.cpp Bitmap.h Box.cpp Box.h CellMesh.cpp CellMesh.h CollisionGrid.cpp
        CollisionGrid.h Cylinder.cpp Cylinder.h DiscGameData.cpp DiscGameData.h
        GameData.cpp GameData.h LevelIndex.cpp
        LevelIndex.h LevelObjects.cpp LevelObjects.h LevelStats.cpp LevelStats.h LightMap.cpp
        LightMap.h Lrz.cpp Lrz.h Png.cpp Png.h PolyList.cpp PolyList.h Rar20.cpp Rar20.h
        SceneNode.cpp SceneNode.h Sprite.cpp Sprite.h SurfaceMesh.cpp SurfaceMesh.h
        TextureCache.h Wav.cpp Wav.h codepages.inc
    """),
    "audio": ("Game sound playback and mixing.", """
        CuePlayback.cpp CuePlayback.h Mixer.cpp Mixer.h OverlapMix.h SpatialSound.h
    """),
    "gfx": ("Renderer-independent sprite and tile geometry.", """
        SpriteState.h SpriteView.cpp SpriteView.h TilePosition.h
    """),
    "save": ("Progress and settings storage.", """
        SaveStore.cpp SaveStore.h SavedGame.cpp SavedGame.h SoundSettings.cpp SoundSettings.h
    """),
    "shell": ("Frontend menus, HUD, and level progress.", """
        Frontend.cpp Frontend.h HudText.cpp HudText.h MenuRender.cpp MenuRender.h
        PauseCaption.cpp PauseCaption.h Progress.cpp Progress.h
    """),
    "sim": ("Gameplay simulation and level setup.", """
        RetailLevel.cpp RetailLevel.h Terrain.cpp Terrain.h World.cpp World.h
    """),
    "web": ("Browser startup, game-data setup, or WebAssembly support.", """
        Browser.cpp Browser.h WebMixer.cpp isolate.js node-mount.js web-audio.js
        web-game-data.mjs webolution.html
    """),
}

RETAIL_UNITS = """
    box_archive compiled_level compiled_level_header level_entity_prefix
    level_gadget_records level_model level_name_tables level_stream object_base_prefix
    sprite_resource texture_resource world_coordinates world_geometry_prefix
""".split()
RETAIL_HEADERS = [*RETAIL_UNITS, "animation_playback"]

OVERLAY_REASONS = {
    ".github/workflows/build-check.yml": "Check Linux and Windows builds on main changes.",
    ".github/workflows/pages.yml": "Build and publish the browser game to GitHub Pages.",
    ".github/workflows/release.yml": "Build and publish desktop releases from a version tag.",
    ".gitignore": "Ignore game data and local build products.",
    "BUILDING.md": "Build the Linux, Windows and browser versions from source.",
    "CMakePresets.json": "Provide native debug, native release and Emscripten release builds.",
    "LICENSE": "MIT license for the exported source.",
    "README.md": "Player setup instructions.",
    "RELEASING.md": "Publish desktop releases on GitHub Actions.",
    "packaging/linux/README.txt": "Player setup instructions included in the Linux archive.",
    "packaging/windows/README.txt": "Player setup instructions included in the Windows zip.",
}

EXCLUDED_ROOTS = set("""
    .agents .claude .codex .git .opencode .rea agent build decomp-lemmings docs old
    release scripts tools web
""".split()) | {"lemmings revolution"}
EXCLUDED_PARTS = {"__test__", "test", "tests", "replays", "fixtures", "harness", "tools"}
EXCLUDED_FILES = {"agents.md", "claude.md"}
PRODUCTION_REPLAY_HEADER = "src/app/Replay.hpp"
DEVELOPMENT_FILE = re.compile(
    r"(?i:(?:^|[._ -])(?:test|replay|fixture|harness|oracle|rea(?:[._ -]|$)|agent(?:[._ -]|$)))"
    r"|(?:Test|Replay|Fixture|Harness|Oracle)(?:[A-Z._ -]|$)"
)
NON_SOURCE_SUFFIXES = set("""
    .7z .a .apk .avi .bin .bmp .box .bz2 .cab .com .dat .deb .dll .dmg .elf .exe
    .gif .gz .ico .img .iso .jar .jpeg .jpg .lib .lvl .lrz .m4a .mov .mp3 .mp4 .msi
    .o .obj .ogg .pak .pdf .png .rar .rpm .so .spr .sys .tar .tgz .wasm .wav .xz
    .zip .zst
""".split())
BINARY_MAGIC = (b"MZ", b"\x7fELF", b"\x00asm", b"LEMB", b"!<arch>\n",
                b"PK\x03\x04", b"PK\x05\x06", b"PK\x07\x08", b"Rar!\x1a\x07",
                b"7z\xbc\xaf\x27\x1c", b"\x1f\x8b", b"BZh", b"\xfd7zXZ\x00",
                b"\x28\xb5\x2f\xfd", b"MSCF", b"\x89PNG\r\n\x1a\n",
                b"\xfe\xed\xfa\xce", b"\xfe\xed\xfa\xcf", b"\xce\xfa\xed\xfe",
                b"\xcf\xfa\xed\xfe", b"\xca\xfe\xba\xbe")
CONTROL = re.compile(rb"[\x00-\x08\x0b\x0c\x0e-\x1f\x7f]")
REFERENCE_EXE = "32675bdb49866e76502713251fdfd2b09078c2481c72ca22ff68ebf4b3737a2f"
HISTORY = re.compile(
    r"\b(?:playtest\w*|workers?|supervisors?)\b"
    r"|\b[a-z][a-z0-9]*(?:-[a-z0-9]+)+-[0-9]{3}\b"
    r"|build[/\\]agent-runs|docs[/\\]scratch|(?<![a-z0-9_])agent[/\\]",
    re.IGNORECASE,
)

# Identify individual platform terms and renderer identifiers that resemble history.
TECHNICAL_TERMS = {
    "CMakeLists.txt": re.compile(r"(?<=-sENVIRONMENT=web,)worker(?=,node\b)"),
    "src/app/BackdropScene.cpp": re.compile(r'(?<=")fluid-pipe-(?:160|164)(?=")'),
    "src/app/RasterRows.h": re.compile(r"\bworker(?= : workers_\b|\.join(?:able)?\(\))"),
    "src/web/webolution.html": re.compile(r"(?<=Service )Worker(?= support\b)|(?<=allow Service )Workers\b"),
}


def is_link(path):
    return path.is_symlink() or path.is_junction()


def check_source(path):
    for entry in (path, *path.parents):
        if entry == ROOT:
            break
        if is_link(entry):
            raise ValueError(f"Source path contains a link: {entry.relative_to(ROOT)}")
    if not path.is_file():
        raise ValueError(f"Source is missing or is not a regular file: {path.relative_to(ROOT)}")


def overlay_files(directory):
    if is_link(directory):
        raise ValueError(f"Overlay path is a link: {directory.relative_to(ROOT)}")
    for path in sorted(directory.iterdir()):
        if path.is_dir() and not is_link(path):
            yield from overlay_files(path)
        else:
            yield path


def manifest():
    entries = {}
    destinations = set()

    def add(source, destination, reason):
        check_source(source)
        if destination.casefold() in destinations:
            raise ValueError(f"Multiple inputs for {destination}")
        destinations.add(destination.casefold())
        entries[destination] = source, reason

    for name, reason in ROOT_FILES.items():
        add(ROOT / name, name, reason)
    for name in CLONE_ICON_HASHES:
        add(ROOT / name, name, "User-supplied clone application icon.")
    for domain, (reason, names) in SOURCE_GROUPS.items():
        for name in names.split():
            path = f"src/{domain}/{name}"
            add(ROOT / path, path, reason)
    for unit in RETAIL_UNITS:
        relative = f"src/{unit}.cpp"
        add(ROOT / "decomp-lemmings" / relative, f"src/assets/retail/{relative}",
            "Compiled production reader for the original file formats.")
    for header in RETAIL_HEADERS:
        relative = f"include/lemmings/{header}.hpp"
        add(ROOT / "decomp-lemmings" / relative, f"src/assets/retail/{relative}",
            "Header used by the production file readers.")
    if is_link(OVERLAY.parent):
        raise ValueError(f"Overlay parent is a link: {OVERLAY.parent.relative_to(ROOT)}")
    for source in overlay_files(OVERLAY):
        path = source.relative_to(OVERLAY).as_posix()
        add(source, path, OVERLAY_REASONS.get(path, "Public branch overlay."))
    return entries


def reset_output():
    for directory in (OUTPUT.parent, OUTPUT):
        if is_link(directory) or directory.resolve() != directory:
            raise ValueError(f"Refusing to write through a redirected path: {directory}")
    if OUTPUT.exists():
        shutil.rmtree(OUTPUT)
    OUTPUT.mkdir(parents=True)


def scan_file(path):
    name = path.relative_to(OUTPUT).as_posix()
    parts = [part.lower() for part in path.relative_to(OUTPUT).parts]
    if (parts[0] in EXCLUDED_ROOTS or EXCLUDED_PARTS.intersection(parts)
            or parts[-1] in EXCLUDED_FILES
            or (name != PRODUCTION_REPLAY_HEADER and DEVELOPMENT_FILE.search(path.stem))
            or (parts[0] == "cmake" and name not in ROOT_FILES)):
        yield f"{name}: excluded path"
    data = path.read_bytes()
    digest = sha256(data).hexdigest()
    if digest == REFERENCE_EXE:
        yield f"{name}: reference executable SHA-256"
    if name in CLONE_ICON_HASHES:
        if digest == CLONE_ICON_HASHES[name]:
            return
        yield f"{name}: unexpected clone icon SHA-256"
    suffixes = {suffix.lower() for suffix in path.suffixes}
    if NON_SOURCE_SUFFIXES.intersection(suffixes | {parts[-1]}):
        yield f"{name}: binary or game-data extension"
    if data.startswith(BINARY_MAGIC) or data[257:262] == b"ustar":
        yield f"{name}: binary signature"
    if CONTROL.search(data):
        yield f"{name}: non-text bytes"
    if b"\r" in data:
        yield f"{name}: non-LF line ending"
    try:
        source = data.decode("utf-8")
    except UnicodeDecodeError:
        yield f"{name}: invalid UTF-8"
        return
    technical_terms = TECHNICAL_TERMS.get(name)
    for number, line in enumerate(source.splitlines(), 1):
        technical_spans = {term.span() for term in technical_terms.finditer(line)} if technical_terms else set()
        for hit in HISTORY.finditer(line):
            if hit.span() not in technical_spans:
                yield f"{name}:{number}: development-history text {hit.group()!r}"


def export():
    entries = manifest()
    reset_output()
    for name, (source, _) in sorted(entries.items()):
        target = OUTPUT / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, target)

    hits = [hit for path in sorted(OUTPUT.rglob("*")) if path.is_file() for hit in scan_file(path)]
    if hits:
        print(*hits, sep="\n")
        print(f"Export scan failed ({len(hits)} hits).")
        return 1
    for name, (_, reason) in sorted(entries.items()):
        print(f"{name}\t{reason}")
    print(f"Export scan passed ({len(entries)} files).")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(export())
    except (OSError, ValueError) as error:
        raise SystemExit(f"Export failed: {error}") from None
