#include "util.hpp"
#include <algorithm>
#include <cstring>

namespace fx::util {
  bool icompare(std::string const& left, std::string const& right) {
    if (left.size() == right.size()) {
      return std::equal(right.begin(), right.end(), left.begin(), left.end(),
                        [](char lchar, char rchar) {
                          return std::tolower(lchar) == std::tolower(rchar);
                        });
    }

    return false;
  }

  std::string lower(std::string const& str) {
    auto copy = str;
    std::transform(copy.begin(), copy.end(), copy.begin(),
                   [](unsigned char cur_char) {
                     return std::tolower(cur_char);
                   });
    return copy;
  }

  fx::result::Result<std::filesystem::path> workspace_descriptor_path() {
    std::filesystem::path workspace;
    auto current_directory = std::filesystem::current_path();
    auto const root_directory = current_directory.root_directory();
    auto const workspace_filename = std::filesystem::path("workspace.fx.yaml");

    while (current_directory.parent_path() != root_directory) {
      auto const possible_workspace = current_directory / workspace_filename;
      if (std::filesystem::exists(possible_workspace)) {
        workspace = possible_workspace;
        break;
      }
      current_directory = current_directory.parent_path();
    }

    if (workspace.empty()) {
      return fx::result::Error(std::string("Workspace descriptor not found."));
    }

    return fx::result::Ok(workspace);
  }

  char** c_vector_string(const std::vector<std::string>& strings) {
    char** result = new char*[strings.size() + 1];

    for (int i = 0; i < strings.size(); ++i) {
      result[i] = new char[strings[i].size() + 1];
      std::memcpy(result[i], strings[i].c_str(), strings[i].size() + 1);
    }

    result[strings.size()] = nullptr;

    return result;
  }
}  // namespace fx::util
