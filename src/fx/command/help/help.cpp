#include "help.hpp"
#include <fmt/core.h>

namespace fx::command {
  Help::Help() = default;

  fx::result::Result<void> Help::run(
      const std::vector<std::string>& /*arguments*/) {
    fmt::print(
        "fx is a workspace tool manager. Learn more: "
        "https://github.com/jathu/fx\n");
    return fx::result::Ok();
  }
}  // namespace fx::command
