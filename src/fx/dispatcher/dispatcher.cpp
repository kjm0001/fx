#include "dispatcher.hpp"
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include "fx/command/forwarder/forwarder.hpp"
#include "fx/command/help/help.hpp"
#include "fx/command/list/list.hpp"
#include "fx/command/version/version.hpp"

namespace fx::dispatcher {
  // Protocol ------------------------------------------------------------------

  Protocol::Protocol() = default;

  Protocol::~Protocol() = default;

  void Protocol::dispatch() {
    const auto result = _command->run(_arguments);
    if (result.failed()) {
      spdlog::error("{0}", result.error());
      exit(1);
    }
  }

  const std::vector<std::string>& Protocol::arguments() {
    return _arguments;
  }

  std::shared_ptr<fx::command::Base> Protocol::command() {
    return _command;
  }

  // Dispatcher ----------------------------------------------------------------

  Dispatcher::Dispatcher(const std::vector<std::string>& arguments) {
    if (arguments.empty() || arguments[0] == "list") {
      _command = std::make_shared<fx::command::List>();
    } else if (arguments[0] == "help" || arguments[0] == "--help" ||
               arguments[0] == "-h") {
      _command = std::make_shared<fx::command::Help>();
    } else if (arguments[0] == "version") {
      _command = std::make_shared<fx::command::Version>();
    } else {
      const auto& command_name = arguments[0];
      _command = std::make_shared<fx::command::Forwarder>(command_name);
      _arguments =
          std::vector<std::string>{arguments.begin() + 1, arguments.end()};
    }
  }
}  // namespace fx::dispatcher
