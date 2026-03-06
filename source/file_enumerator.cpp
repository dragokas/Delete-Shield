#include "file_enumerator.h"
#include <stack>
#include <iostream>

namespace fs = std::filesystem;

Generator<std::filesystem::path> FileEnumerator::Search(std::filesystem::path rootDir, bool recursive)
{
    if (!fs::exists(rootDir) || !fs::is_directory(rootDir)) {
        std::cerr << "Invalid root path: " << rootDir << std::endl;
        co_return;
    }

    std::stack<fs::path> dirs;
    dirs.push(rootDir);

    while (!dirs.empty()) {
        fs::path currentDir = dirs.top();
        dirs.pop();

        for (const auto& entry : fs::directory_iterator(currentDir)) {
            try {
                if (fs::is_symlink(entry.status())) {
                    continue;
                }
                if (fs::is_directory(entry.status())) {
                    dirs.push(entry.path());
                    if (recursive)
                    {
                        co_yield entry.path();
                    }
                }
                else if (fs::is_regular_file(entry.status())) {
                    co_yield entry.path();
                }
            }
            catch (const fs::filesystem_error& e) {
                std::cerr << "Error accessing " << entry.path()
                    << ": " << e.what() << std::endl;
            }
        }
    }
}