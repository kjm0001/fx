#pragma once

#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>
#include <variant>
#include "fx/result/result.hpp"

namespace fx::parser::yaml_to_json {
  fx::result::Result<void> convert(const YAML::Node& yaml,
                                   nlohmann::json& json);

  void parse_map(const YAML::Node& yaml, nlohmann::json& json);

  void parse_sequence(const YAML::Node& yaml, nlohmann::json& json);

  std::variant<bool, int64_t, double, std::string> parse_scalar(
      const YAML::Node& yaml);
}  // namespace fx::parser::yaml_to_json
