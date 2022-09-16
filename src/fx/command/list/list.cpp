#include "list.hpp"
#include <fmt/color.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <algorithm>
#include <set>
#include <unordered_map>
#include "fx/parser/parser.hpp"
#include "fx/util/util.hpp"

namespace fx::command {
  List::List() = default;

  static const std::vector<std::tuple<std::string, std::string>>
      standard_commands{
          {"list", "List available commands."},
          {"help", "Learn more about fx."},
          {"version", "Print the fx version."},
      };

  static const std::string spacing{"    "};

  fx::result::Result<void> List::run(
      const std::vector<std::string>& /*arguments*/) {
    fmt::print("{0} — workspace tool manager [version {1}]\n\n",
               fmt::format(fmt::emphasis::bold, "fx"), FX_VERSION);
    fmt::print("Usage:  fx <command> --help\n");
    fmt::print("        fx <command> <options...> <args...>\n\n");

    fmt::print("[workspace fx]\n");
    for (const auto& [command_name, description] : standard_commands) {
      fmt::print("{0}{1} - {2}\n", spacing, command_name, description);
    }

    const auto workspace_path_result = fx::util::workspace_descriptor_path();
    if (workspace_path_result.failed()) {
      fmt::print(fg(fmt::terminal_color::yellow),
                 "\nYou are not in a workspace.\n");
    } else {
      const auto& workspace_path = workspace_path_result.value();
      fmt::print("\n[workspace {0}]\n", workspace_path.u8string());

      const auto workspace_result =
          fx::parser::parse_workspace_descriptor(workspace_path);
      if (workspace_result.failed()) {
        return fx::result::Error(workspace_result.error());
      } else {
        const auto workspace = workspace_result.value();
        const auto commands =
            list_workspace_commands(workspace_path, workspace);
        for (const auto& command : commands) {
          fmt::print("{0}{1} - {2}\n", spacing, command.command_name,
                     command.synopsis);
        }
      }
    }

    return fx::result::Ok();
  }

  std::vector<search_result_t> List::list_workspace_commands(
      const std::filesystem::path& workspace_descriptor_path,
      const fx::descriptor::v1beta::FxWorkspaceDescriptor& workspace) {
    std::vector<search_result_t> commands;
    const auto descriptor_paths =
        list_command_descriptor_paths(workspace_descriptor_path, workspace);
    const auto workspace_directory_size =
        workspace_descriptor_path.parent_path().u8string().size() + 1;

    for (auto const& descriptor_path : descriptor_paths) {
      search_result_t search_result;
      search_result.command_name =
          descriptor_path.parent_path().u8string().replace(
              0, workspace_directory_size, "");
      const auto descriptor_result =
          fx::parser::parse_command_descriptor(descriptor_path);
      if (descriptor_result.ok()) {
        search_result.synopsis = descriptor_result.value().synopsis();
      } else {
        search_result.synopsis =
            fmt::format(fg(fmt::terminal_color::red),
                        "Descriptor contains errors. Run this to learn more.",
                        search_result.command_name);
      }
      commands.emplace_back(search_result);
    }

    std::sort(commands.begin(), commands.end(),
              [](const auto& left, const auto& right) {
                return left.command_name < right.command_name;
              });

    return commands;
  }

  std::vector<std::filesystem::path> List::list_command_descriptor_paths(
      const std::filesystem::path& workspace_descriptor_path,
      const fx::descriptor::v1beta::FxWorkspaceDescriptor& workspace) {
    std::vector<std::filesystem::path> descriptor_paths;

    std::set<std::filesystem::path> paths_to_ignore;
    for (const auto& ignore : workspace.ignore()) {
      paths_to_ignore.insert((workspace_descriptor_path.parent_path() /
                              std::filesystem::path(ignore))
                                 .lexically_normal());
    }

    if (!workspace_descriptor_path.empty()) {
      std::filesystem::recursive_directory_iterator current_entry(
          workspace_descriptor_path.parent_path());
      const std::filesystem::recursive_directory_iterator end_entry;
      for (; current_entry != end_entry; current_entry++) {
        auto const filename = current_entry->path().filename();
        if (filename == "command.fx.yaml") {
          descriptor_paths.push_back(current_entry->path());
        } else if ((current_entry->is_directory() &&
                    strncmp(filename.c_str(), ".", 1) == 0) ||
                   (paths_to_ignore.find(current_entry->path()) !=
                    paths_to_ignore.end())) {
          spdlog::debug("Ignoring command search in {0}",
                        current_entry->path().u8string());
          current_entry.disable_recursion_pending();
        }
      }
    }

    return descriptor_paths;
  }
}  // namespace fx::command
