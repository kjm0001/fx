#pragma once

#include <google/protobuf/util/json_util.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "fx/descriptor/v1beta/descriptor.pb.h"
#include "fx/result/result.hpp"

namespace fx::parser {
  template <typename D>
  fx::result::Result<D> parse_descriptor(
      const std::filesystem::path& descriptor_path);

  template <typename D>
  fx::result::Result<D> json_to_descriptor(const nlohmann::json& json);

  fx::result::Result<fx::descriptor::v1beta::FxWorkspaceDescriptor>
  parse_workspace_descriptor(const std::filesystem::path& descriptor_path);

  fx::result::Result<fx::descriptor::v1beta::FxCommandDescriptor>
  parse_command_descriptor(const std::filesystem::path& descriptor_path);
}  // namespace fx::parser
