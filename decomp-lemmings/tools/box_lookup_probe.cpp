#include "lemmings/box_archive.hpp"

#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc < 2) return 2;
    try {
        std::vector<lemmings::BoxArchiveIndex> archives;
        for (int i = 1; i < argc; ++i) {
            archives.push_back(lemmings::read_box_archive_index(argv[i]));
        }
        std::string name;
        while (std::getline(std::cin, name)) {
            const auto found = lemmings::find_box_entry(archives, name);
            if (found) {
                std::cout << found->archive << ' ' << found->entry << '\n';
            } else {
                std::cout << "-1 -1\n";
            }
        }
        return std::cout ? 0 : 2;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 2;
    }
}
