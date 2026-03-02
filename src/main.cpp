#include "core/site_builder.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

namespace {

fs::path find_project_root(fs::path start) {
    std::error_code ec;
    if (!fs::exists(start, ec)) {
        return {};
    }
    start = fs::absolute(start, ec);
    if (ec) {
        return {};
    }

    fs::path current = start;
    while (!current.empty()) {
        if (fs::exists(current / "blogpp.yml", ec)) {
            return current;
        }
        if (fs::exists(current / "xmake.lua", ec) && fs::exists(current / "src", ec)) {
            return current;
        }
        auto parent = current.parent_path();
        if (parent == current) {
            break;
        }
        current = parent;
    }
    return {};
}

void print_help() {
    std::cout << "BlogPlusPlus (C++ + XMake)\n"
              << "Usage:\n"
              << "  blogpp init                 Bootstrap config and content skeleton\n"
              << "  blogpp build                Build static site\n"
              << "  blogpp serve [port]         Build and run static server (default 4000)\n"
              << "  blogpp new <title>          Create a new post markdown file\n"
              << "  blogpp help                 Show help\n";
}

std::string merge_args(int argc, char** argv, int begin_index) {
    std::ostringstream oss;
    for (int i = begin_index; i < argc; ++i) {
        if (i > begin_index) {
            oss << " ";
        }
        oss << argv[i];
    }
    return oss.str();
}

}  // namespace

int main(int argc, char** argv) {
    fs::path root = find_project_root(fs::current_path());
    if (root.empty() && argc > 0) {
        root = find_project_root(fs::path(argv[0]).parent_path());
    }
    if (root.empty()) {
        root = fs::current_path();
    }

    blogpp::SiteBuilder builder(root);
    if (argc < 2) {
        print_help();
        return 0;
    }

    const std::string cmd = argv[1];
    if (cmd == "build") {
        return builder.build() ? 0 : 1;
    }
    if (cmd == "init") {
        return builder.init_project() ? 0 : 1;
    }
    if (cmd == "serve") {
        int port = 4000;
        if (argc >= 3) {
            try {
                port = std::stoi(argv[2]);
            } catch (...) {
                std::cerr << "Invalid port: " << argv[2] << "\n";
                return 1;
            }
        }
        return builder.serve(port) ? 0 : 1;
    }
    if (cmd == "new") {
        if (argc < 3) {
            std::cerr << "Please provide a title, e.g. blogpp new \"Hello Blog\"\n";
            return 1;
        }
        const auto title = merge_args(argc, argv, 2);
        return builder.create_post(title) ? 0 : 1;
    }
    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        print_help();
        return 0;
    }

    std::cerr << "Unknown command: " << cmd << "\n";
    print_help();
    return 1;
}
