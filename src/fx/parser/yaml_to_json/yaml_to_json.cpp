#include "yaml_to_json.hpp"
#include <fmt/core.h>

namespace fx::parser::yaml_to_json {
  fx::result::Result<void> convert(const YAML::Node& yaml,
                                   nlohmann::json& json) {
    if (yaml.Type() == YAML::NodeType::Map) {
      parse_map(yaml, json);
    } else if (yaml.Type() == YAML::NodeType::Sequence) {
      parse_sequence(yaml, json);
    } else {
      const std::string type_name = [&]() {
        switch (yaml.Type()) {
          case YAML::NodeType::Undefined:
            return "undefined";
          case YAML::NodeType::Null:
            return "empty";
          case YAML::NodeType::Scalar:
            return "scalar";
          default:
            return "unknown";
        }
      }();
      return fx::result::Error(
          fmt::format("The root YAML type is: {0}. The root YAML object must "
                      "be a map or an array.",
                      type_name));
    }

    return fx::result::Ok();
  }

  void parse_map(const YAML::Node& yaml, nlohmann::json& json) {
    json = nlohmann::json::object();

    for (auto&& child_tuple : yaml) {
      const auto key = child_tuple.first.as<std::string>();
      const auto child_node = child_tuple.second;

      if (child_node.Type() == YAML::NodeType::Scalar) {
        const auto value_variant = parse_scalar(child_node);
        std::visit(
            [&](auto value) {
              json[key] = value;
            },
            value_variant);
      } else if (child_node.Type() == YAML::NodeType::Sequence ||
                 child_node.Type() == YAML::NodeType::Map) {
        convert(child_node, json[key]);
      }
    }
  }

  void parse_sequence(const YAML::Node& yaml, nlohmann::json& json) {
    json = nlohmann::json::array();

    for (auto&& child_node : yaml) {
      if (child_node.Type() == YAML::NodeType::Scalar) {
        const auto value_variant = parse_scalar(child_node);
        std::visit(
            [&](auto value) {
              json.emplace_back(value);
            },
            value_variant);
      } else if (child_node.Type() == YAML::NodeType::Sequence ||
                 child_node.Type() == YAML::NodeType::Map) {
        nlohmann::json child_json;
        convert(child_node, child_json);
        json.emplace_back(child_json);
      }
    }
  }

  std::variant<bool, int64_t, double, std::string> parse_scalar(
      const YAML::Node& yaml) {
    std::variant<bool, int64_t, double, std::string> value;
    bool bool_scalar;
    int64_t int_scalar;
    double double_scalar;
    std::string string_scalar;

    if (YAML::convert<bool>::decode(yaml, bool_scalar)) {
      value = bool_scalar;
    } else if (YAML::convert<int64_t>::decode(yaml, int_scalar)) {
      value = int_scalar;
    } else if (YAML::convert<double>::decode(yaml, double_scalar)) {
      value = double_scalar;
    } else if (YAML::convert<std::string>::decode(yaml, string_scalar)) {
      value = string_scalar;
    }

    return value;
  }
}  // namespace fx::parser::yaml_to_json
