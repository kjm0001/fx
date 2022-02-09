#include <fmt/color.h>
#include <fmt/core.h>
#include <spdlog/cfg/env.h>
#include <spdlog/spdlog.h>
#include <memory>
#include <string>
#include "fx/dispatcher/dispatcher.hpp"

#define FX_ASCII_ART                                                          \
  " ______   __  __\n/\\  ___\\ /\\_\\_\\_\\\n\\ \\  __\\ \\/_/\\_\\/_\n \\ " \
  "\\_\\     /\\_\\/\\_\\\n  \\/_/     \\/_/\\/_/\n"

void setup_logger() {
  spdlog::cfg::load_env_levels();
  const auto prefix = fmt::format(fmt::emphasis::faint, "fx %H:%M:%S.%e");
  const auto pid = fmt::format(fg(fmt::terminal_color::magenta), "%P");
  const auto thread = fmt::format(fmt::emphasis::faint, "~%t");
  const auto separator = fmt::format(fmt::emphasis::faint, ":");
  spdlog::set_pattern(
      fmt::format("{0} %^%5l%$ {1}{2}{3} %v", prefix, pid, thread, separator));
  spdlog::debug("starting fx v{0}\n{1}", FX_VERSION, FX_ASCII_ART);
}

int main(int argc, char *argv[]) {  // NOLINT(bugprone-exception-escape)
  setup_logger();

  const auto dispatcher = std::make_unique<fx::dispatcher::Dispatcher>(
      std::vector<std::string>{argv + 1, argv + argc});
  dispatcher->dispatch();
}
