#include "parser.hpp"
#include <fmt/core.h>
#include "fx/parser/validator/validator.hpp"
#include "fx/parser/yaml_to_json/yaml_to_json.hpp"

namespace fx::parser {
  template <typename D>
  fx::result::Result<D> parse_descriptor(
      const std::filesystem::path& descriptor_path) {
    YAML::Node yaml;
    try {
      yaml = YAML::LoadFile(descriptor_path);
    } catch (...) {
      return fx::result::Error(
          fmt::format("Unable to read {0}.", descriptor_path.u8string()));
    }

    nlohmann::json json;
    const auto json_result = fx::parser::yaml_to_json::convert(yaml, json);
    if (json_result.failed()) {
      return fx::result::Error(fmt::format("Invalid descriptor: {0}. {1}",
                                           descriptor_path.u8string(),
                                           json_result.error()));
    }

    const auto descriptor_result = json_to_descriptor<D>(json);
    if (descriptor_result.failed()) {
      return fx::result::Error(fmt::format("Invalid descriptor: {0}. {1}",
                                           descriptor_path.u8string(),
                                           descriptor_result.error()));
    }

    const auto validation_result =
        fx::parser::validator::validate_descriptor(descriptor_result.value());
    if (validation_result.failed()) {
      return fx::result::Error(fmt::format("Invalid descriptor: {0}. {1}",
                                           descriptor_path.u8string(),
                                           validation_result.error()));
    }

    return fx::result::Ok(descriptor_result.value());
  }

  template <typename D>
  fx::result::Result<D> json_to_descriptor(const nlohmann::json& json) {
    D descriptor;
    const auto status = google::protobuf::util::JsonStringToMessage(
        json.dump(), &descriptor, {});

    if (!status.ok()) {
      return fx::result::Error(status.ToString());
    }

    return fx::result::Ok(descriptor);
  }

  fx::result::Result<fx::descriptor::v1beta::FxWorkspaceDescriptor>
  parse_workspace_descriptor(const std::filesystem::path& descriptor_path) {
    return parse_descriptor<fx::descriptor::v1beta::FxWorkspaceDescriptor>(
        descriptor_path);
  }

  fx::result::Result<fx::descriptor::v1beta::FxCommandDescriptor>
  parse_command_descriptor(const std::filesystem::path& descriptor_path) {
    return parse_descriptor<fx::descriptor::v1beta::FxCommandDescriptor>(
        descriptor_path);
  }
}  // namespace fx::parser
