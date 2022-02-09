#pragma once

#include <filesystem>
#include <tuple>
#include "fx/command/base/base.hpp"
#include "fx/descriptor/v1beta/descriptor.pb.h"
#include "fx/result/result.hpp"

namespace fx::command {
  namespace {
    struct search_result_t {
      std::string command_name;
      std::string synopsis;
    };
  }  // namespace

  class List : public fx::command::Base {
   public:
    List();
    fx::result::Result<void> run(
        const std::vector<std::string>& arguments) override;

   private:
    static std::vector<search_result_t> list_workspace_commands(
        const std::filesystem::path& workspace_descriptor_path,
        const fx::descriptor::v1beta::FxWorkspaceDescriptor& workspace);

    static std::vector<std::filesystem::path> list_command_descriptor_paths(
        const std::filesystem::path& workspace_descriptor_path,
        const fx::descriptor::v1beta::FxWorkspaceDescriptor& workspace);
  };
}  // namespace fx::command
