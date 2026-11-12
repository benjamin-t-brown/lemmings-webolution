#include "lemmings/compiled_level.hpp"

#include <stdexcept>

namespace lemmings {

CompiledLevel read_compiled_level(std::span<const std::uint8_t> bytes,
                                 bool require_eof) {
    LevelByteStream stream{bytes};
    CompiledLevel result;
    const char* section = "prefix";
    try {
        result.prefix = read_compiled_level_prefix(stream);
        section = "world geometry";
        result.world = read_world_geometry_prefix(stream);
        section = "paired objects";
        result.objects = read_object_base_prefix(stream);
        section = "entity tables";
        result.entities = read_level_entity_prefix(stream);
        section = "gadget records";
        result.gadgets = read_level_gadget_records(stream);
    } catch (const std::invalid_argument& error) {
        throw std::invalid_argument(std::string(section) + " at byte " +
            std::to_string(stream.cursor) + ": " + error.what());
    }
    result.consumed_bytes = stream.cursor;
    if (require_eof && stream.cursor != bytes.size()) {
        throw std::invalid_argument("trailing level data at byte " +
            std::to_string(stream.cursor) + " (" +
            std::to_string(bytes.size() - stream.cursor) + " bytes remain)");
    }
    return result;
}

ArchivedCompiledLevel load_compiled_level(
    const std::vector<BoxArchiveIndex>& archives, const std::string& name) {
    const auto location = find_box_entry(archives, name);
    if (!location) throw std::invalid_argument("level not found in archives: " + name);
    const auto& archive = archives[location->archive];
    const auto& entry = archive.entries[location->entry];
    auto bytes = archive.read_entry(location->entry);
    try {
        return {archive.path, entry.name, entry.size, read_compiled_level(bytes)};
    } catch (const std::invalid_argument& error) {
        throw std::invalid_argument(entry.name + ": " + error.what());
    }
}

} // namespace lemmings
