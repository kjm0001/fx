#include "forwarder.hpp"
#include <fmt/core.h>
#include <pwd.h>
#include <spdlog/spdlog.h>
#include <unistd.h>
#include "fx/argparse/argparse.hpp"
#include "fx/command/forwarder/help/help.hpp"
#include "fx/parser/parser.hpp"
#include "fx/util/util.hpp"

extern char** environ;

namespace fx::command {
  // Protocol ------------------------------------------------------------------

  Protocol::~Protocol() = default;

  // Forwarder -----------------------------------------------------------------

  Forwarder::Forwarder(const std::string& command_name)
      : _command_name(std::move(command_name)){};

  fx::result::Result<void> Forwarder::run(
      const std::vector<std::string>& arguments) {
    const auto workspace_path_result = find_workspace_descriptor_path();
    if (workspace_path_result.failed()) {
      return fx::result::Error(workspace_path_result.error());
    }
    const auto& workspace_path = workspace_path_result.value();

    const auto descriptor_result = parse_command_descriptor(workspace_path);
    if (descriptor_result.failed()) {
      return fx::result::Error(descriptor_result.error());
    }
    const auto& descriptor = descriptor_result.value();

    const auto command_arguments_result =
        parse_command_arguments(descriptor, arguments);
    if (command_arguments_result.failed()) {
      return fx::result::Error(command_arguments_result.error());
    }
    const auto& command_arguments = command_arguments_result.value();

    if (command_arguments["help"]["value"]) {
      return execute_help(descriptor);
    } else {
      const std::vector<std::string> execution_arguments =
          collate_execution_arguments(descriptor, command_arguments);
      const std::vector<std::string> enviornment_variables =
          collate_enviornment_variables(workspace_path);
      return execute_command(execution_arguments, enviornment_variables);
    }
  }

  fx::result::Result<std::filesystem::path>
  Forwarder::find_workspace_descriptor_path() {
    return fx::util::workspace_descriptor_path();
  }

  fx::result::Result<fx::descriptor::v1beta::FxCommandDescriptor>
  Forwarder::parse_command_descriptor(
      const std::filesystem::path& workspace_descriptor_path) {
    const auto command_descriptor_path =
        workspace_descriptor_path.parent_path() /
        std::filesystem::path(_command_name) /
        std::filesystem::path("command.fx.yaml");

    if (!std::filesystem::exists(command_descriptor_path)) {
      return fx::result::Error(
          fmt::format("Unknown command \"{0}\".", _command_name));
    }

    const auto result =
        fx::parser::parse_command_descriptor(command_descriptor_path);
    if (result.failed()) {
      return fx::result::Error(result.error());
    }

    return fx::result::Ok(result.value());
  }

  fx::result::Result<nlohmann::json> Forwarder::parse_command_arguments(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      const std::vector<std::string>& arguments) {
    const auto result = fx::argparse::parse(arguments, descriptor);
    if (result.failed()) {
      return fx::result::Error(result.error());
    }
    return fx::result::Ok(result.value());
  }

  std::string Forwarder::shell() {
    return std::string(getpwuid(geteuid())->pw_shell);
  }

  std::vector<std::string> Forwarder::collate_execution_arguments(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      const nlohmann::json& arguments) {
    const auto invocation =
        fmt::format("{0} '{1}'", descriptor.runtime().run(), arguments.dump());
    return std::vector<std::string>{shell(), "-l", "-c", invocation};
  }

  std::vector<std::string> Forwarder::collate_enviornment_variables(
      const std::filesystem::path& workspace_descriptor_path) {
    std::vector<std::string> envvars{
        fmt::format("FX_WORKSPACE_DIRECTORY={0}",
                    workspace_descriptor_path.parent_path().u8string())};

    char** current_envvar_pointer = environ;
    for (; *current_envvar_pointer != nullptr; current_envvar_pointer++) {
      envvars.emplace_back(*current_envvar_pointer);
    }

    return envvars;
  }

  fx::result::Result<void> Forwarder::execute_command(
      const std::vector<std::string>& arguments,
      const std::vector<std::string>& envvars) {
    char** c_arguments = fx::util::c_vector_string(arguments);
    char** c_envvars = fx::util::c_vector_string(envvars);

    spdlog::debug("Executing: {0}", fmt::join(arguments, " "));
    execve(c_arguments[0], const_cast<char* const*>(c_arguments), c_envvars);

    free(c_arguments);
    free(c_envvars);
    return fx::result::Error(fmt::format("Error executing command."));
  }

  fx::result::Result<void> Forwarder::execute_help(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    return fx::command::forwarder::help::print(_command_name, descriptor);
  }
}  // namespace fx::command
