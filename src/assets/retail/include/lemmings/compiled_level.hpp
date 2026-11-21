#pragma once

#include "lemmings/box_archive.hpp"
#include "lemmings/level_entity_prefix.hpp"
#include "lemmings/level_gadget_records.hpp"
#include "lemmings/level_name_tables.hpp"
#include "lemmings/object_base_prefix.hpp"
#include "lemmings/world_geometry_prefix.hpp"

namespace lemmings {

// New tooling API composing the recovered serialized readers in loader order.
// This is not the original in-memory world layout or a playable game state.
// Unknown fields retain their existing names and floating point bit patterns.
struct CompiledLevel {
    CompiledLevelPrefix prefix;
    WorldGeometryPrefix world;
    ObjectBasePrefix objects;
    LevelEntityPrefix entities;
    LevelGadgetRecords gadgets;
    std::size_t consumed_bytes = 0;
};

// Strict EOF is a tooling check, not a recovered original loader behavior.
// Disable it only when parsing a level at the beginning of a larger buffer.
// Unsupported variants and truncated inputs throw std::invalid_argument.
CompiledLevel read_compiled_level(std::span<const std::uint8_t> bytes,
                                 bool require_eof = true);

struct ArchivedCompiledLevel {
    std::filesystem::path archive_path;
    std::string entry_name;
    std::size_t entry_size = 0;
    CompiledLevel level;
};

// Uses the recovered archive lookup: callers put PATCH.BOX before LEMMINGS.BOX.
// Reads the selected entry in place; does not extract assets to the filesystem.
ArchivedCompiledLevel load_compiled_level(
    const std::vector<BoxArchiveIndex>& archives, const std::string& name);

} // namespace lemmings
