#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include "fx/result/result.hpp"

// Utility functions that don't quite fit somewhere specific. I know, I know.
namespace fx::util {
  bool icompare(std::string const& left, std::string const& right);

  std::string lower(std::string const& str);

  fx::result::Result<std::filesystem::path> workspace_descriptor_path();

  char** c_vector_string(const std::vector<std::string>& strings);
}  // namespace fx::util
