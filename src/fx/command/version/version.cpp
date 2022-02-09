#include "version.hpp"
#include <fmt/core.h>

namespace fx::command {
  Version::Version() = default;

  fx::result::Result<void> Version::run(
      const std::vector<std::string>& /*arguments*/) {
    fmt::print("fx-{0}\n", FX_VERSION);
    return fx::result::Ok();
  }
}  // namespace fx::command
