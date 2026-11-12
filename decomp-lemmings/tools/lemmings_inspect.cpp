#include "lemmings/compiled_level.hpp"

#include <iostream>
#include <set>
#include <stdexcept>
#include <string_view>

namespace {
void json_string(std::string_view value) {
    constexpr char hex[] = "0123456789abcdef";
    std::cout << '"';
    for (const unsigned char c : value) {
        if (c == '"' || c == '\\') std::cout << '\\' << static_cast<char>(c);
        else if (c < 0x20 || c >= 0x80) {
            // Archive strings are bytes, not established UTF-8. Preserve each
            // non-ASCII byte as a JSON Unicode escape without guessing a codec.
            std::cout << "\\u00" << hex[c >> 4] << hex[c & 15];
        } else std::cout << static_cast<char>(c);
    }
    std::cout << '"';
}

void usage(std::ostream& output) {
    output << "Usage: lemmings_inspect --archive <lemmings.box> [--patch <patch.box>]\n"
              "                        (--list-levels | --level <archive-entry-name>)\n"
              "Outputs JSON. Patch entries take precedence. No assets are extracted.\n";
}

bool is_level(const std::string& name) {
    if (name.size() < 4) return false;
    std::string suffix = name.substr(name.size() - 4);
    for (auto& c : suffix) if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
    return suffix == ".LVL";
}

void write_location(const lemmings::BoxArchiveIndex& archive,
                    const lemmings::BoxArchiveEntry& entry) {
    std::cout << "{\"name\":";
    json_string(entry.name);
    std::cout << ",\"archive\":";
    json_string(archive.path.generic_string());
    std::cout << ",\"bytes\":" << entry.size << '}';
}

void list_levels(const std::vector<lemmings::BoxArchiveIndex>& archives) {
    std::set<std::string> seen;
    bool first = true;
    std::cout << "{\"levels\":[";
    for (const auto& archive : archives) {
        for (const auto& entry : archive.entries) {
            if (!is_level(entry.name) || !seen.insert(entry.name).second) continue;
            if (!first) std::cout << ',';
            first = false;
            write_location(archive, entry);
        }
    }
    std::cout << "],\"count\":" << seen.size() << "}\n";
}

void summarize(const lemmings::ArchivedCompiledLevel& loaded) {
    const auto& level = loaded.level;
    const auto& prefix = level.prefix;
    const auto& gadgets = level.gadgets;
    std::cout << "{\"name\":";
    json_string(loaded.entry_name);
    std::cout << ",\"archive\":";
    json_string(loaded.archive_path.generic_string());
    std::cout << ",\"bytes\":" << loaded.entry_size
              << ",\"consumed_bytes\":" << level.consumed_bytes
              << ",\"header\":{\"marker_matches\":"
              << (prefix.header.marker_matches ? "true" : "false")
              << ",\"fields\":[";
    for (std::size_t i = 0; i < prefix.header.fields.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << prefix.header.fields[i];
    }
    std::cout << "],\"flags\":[";
    for (std::size_t i = 0; i < prefix.header.flags.size(); ++i) {
        if (i) std::cout << ',';
        std::cout << (prefix.header.flags[i] ? "true" : "false");
    }
    std::cout << "]},\"name_table_counts\":[" << prefix.first_names.size() << ','
              << prefix.second_names.size() << ',' << prefix.third_names.size() << ','
              << prefix.fourth_names.size() << ',' << prefix.fifth_names.size()
              << "],\"world_names\":[";
    json_string(level.world.first_name);
    std::cout << ',';
    json_string(level.world.second_name);
    std::cout << "],\"object_children\":[" << level.objects.children.size() << ','
              << level.objects.second_object_children.size()
              << "],\"entity_counts\":{\"registry_size\":" << level.entities.registry_size
              << ",\"groups\":" << level.entities.groups.size()
              << ",\"name_pairs\":" << level.entities.name_pairs.size()
              << ",\"actors\":" << level.entities.actors.size()
              << ",\"subclass_actors\":" << level.entities.subclass_actors.size()
              << ",\"basic\":" << gadgets.basic_entities.size()
              << ",\"linked\":" << gadgets.linked_entities.size()
              << ",\"reader_425e50\":" << gadgets.entities_425e50.size()
              << ",\"reader_426c6b\":" << gadgets.entities_426c6b.size()
              << ",\"spatial\":" << gadgets.spatial_entities.size()
              << ",\"reader_424187\":" << gadgets.entities_424187.size()
              << ",\"reader_42acac\":" << gadgets.entities_42acac.size()
              << ",\"reader_4347ec\":" << gadgets.entities_4347ec.size()
              << ",\"reader_4342bf\":" << gadgets.basic_4342bf.size()
              << ",\"reader_41e038\":" << gadgets.spatial_41e038.size()
              << ",\"tagged\":" << gadgets.tagged_entities.size()
              << ",\"named\":" << gadgets.named_entities.size()
              << "},\"final_record_count\":" << gadgets.final_records.size() << "}\n";
}
} // namespace

int main(int argc, char** argv) {
    try {
        std::string archive_path, patch_path, level_name;
        bool list = false;
        for (int i = 1; i < argc; ++i) {
            const std::string option = argv[i];
            if (option == "--help" || option == "-h") { usage(std::cout); return 0; }
            if (option == "--list-levels") {
                if (list) throw std::invalid_argument("duplicate --list-levels");
                list = true;
                continue;
            }
            std::string* destination = nullptr;
            if (option == "--archive") destination = &archive_path;
            else if (option == "--patch") destination = &patch_path;
            else if (option == "--level") destination = &level_name;
            else throw std::invalid_argument("unknown option: " + option);
            if (!destination->empty()) throw std::invalid_argument("duplicate option: " + option);
            if (++i == argc || std::string_view(argv[i]).starts_with("--")) {
                throw std::invalid_argument("missing value for " + option);
            }
            *destination = argv[i];
        }
        if (archive_path.empty() || list == !level_name.empty()) {
            throw std::invalid_argument("provide --archive and exactly one of --list-levels or --level");
        }
        std::vector<lemmings::BoxArchiveIndex> archives;
        if (!patch_path.empty()) archives.push_back(lemmings::read_box_archive_index(patch_path));
        archives.push_back(lemmings::read_box_archive_index(archive_path));
        if (list) list_levels(archives);
        else summarize(lemmings::load_compiled_level(archives, level_name));
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "lemmings_inspect: " << error.what() << '\n';
        return 1;
    }
}
