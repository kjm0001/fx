#include "fx/parser/parser.hpp"
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <nlohmann/json.hpp>
#include "fx/result/result.hpp"

// ParseWorkspaceDescriptor ----------------------------------------------------

struct ParseWorkspaceDescriptor : testing::Test {
  void static expect_parse_eq(
      const std::filesystem::path& descriptor_path,
      const fx::descriptor::v1beta::FxWorkspaceDescriptor& expected) {
    const auto result = fx::parser::parse_workspace_descriptor(descriptor_path);
    ASSERT_TRUE(result.ok()) << result.error();
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(
        expected, result.value()));
  }

  void static expect_parse_fail(const std::filesystem::path& descriptor_path,
                                const std::string& expected_error) {
    const auto result = fx::parser::parse_workspace_descriptor(descriptor_path);
    ASSERT_TRUE(result.failed());
    EXPECT_EQ(expected_error, result.error());
  }
};

TEST_F(ParseWorkspaceDescriptor, NonexistentYaml) {
  const std::filesystem::path descriptor_path("nonexistent.yaml");

  expect_parse_fail(descriptor_path, "Unable to read nonexistent.yaml.");
}

TEST_F(ParseWorkspaceDescriptor, InvalidYamlToJson) {
  const std::filesystem::path descriptor_path(
      "test/fx/parser/__data__/invalid_json.workspace.fx.yaml");

  expect_parse_fail(
      descriptor_path,
      "Invalid descriptor: "
      "test/fx/parser/__data__/invalid_json.workspace.fx.yaml. The "
      "root YAML type is: "
      "empty. The root YAML object must be a map or an array.");
}

TEST_F(ParseWorkspaceDescriptor, InvalidJsonToDescriptor) {
  const std::filesystem::path descriptor_path(
      "test/fx/parser/__data__/invalid_descriptor.workspace.fx.yaml");

  expect_parse_fail(
      descriptor_path,
      "Invalid descriptor: "
      "test/fx/parser/__data__/invalid_descriptor.workspace.fx.yaml. "
      "INVALID_ARGUMENT:invalid_field: Cannot find field.");
}

TEST_F(ParseWorkspaceDescriptor, ValidationFail) {
  const std::filesystem::path descriptor_path(
      "test/fx/parser/__data__/validation_fail.workspace.fx.yaml");

  expect_parse_fail(
      descriptor_path,
      "Invalid descriptor: "
      "test/fx/parser/__data__/validation_fail.workspace.fx.yaml. Unsupported "
      "descriptor "
      "version \"fake-version\", only v1beta is supported.");
}

TEST_F(ParseWorkspaceDescriptor, ValidYaml) {
  const std::filesystem::path descriptor_path(
      "test/fx/parser/__data__/valid.workspace.fx.yaml");

  fx::descriptor::v1beta::FxWorkspaceDescriptor expected;
  expected.set_descriptor_version("v1beta");

  expect_parse_eq(descriptor_path, expected);
}

// ParseCommandDescriptor ------------------------------------------------------

struct ParseCommandDescriptor : testing::Test {
  void static expect_parse_eq(
      const std::filesystem::path& descriptor_path,
      const fx::descriptor::v1beta::FxCommandDescriptor& expected) {
    const auto result = fx::parser::parse_command_descriptor(descriptor_path);
    ASSERT_TRUE(result.ok()) << result.error();
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(
        expected, result.value()));
  }

  void static expect_parse_fail(const std::filesystem::path& descriptor_path,
                                const std::string& expected_error) {
    const auto result = fx::parser::parse_command_descriptor(descriptor_path);
    ASSERT_TRUE(result.failed());
    EXPECT_EQ(expected_error, result.error());
  }
};

TEST_F(ParseCommandDescriptor, NonexistentYaml) {
  const std::filesystem::path descriptor_path("nonexistent.yaml");

  expect_parse_fail(descriptor_path, "Unable to read nonexistent.yaml.");
}

TEST_F(ParseCommandDescriptor, InvalidYamlToJson) {
  const std::filesystem::path descriptor_path(
      "test/fx/parser/__data__/invalid_json.command.fx.yaml");

  expect_parse_fail(descriptor_path,
                    "Invalid descriptor: "
                    "test/fx/parser/__data__/invalid_json.command.fx.yaml. The "
                    "root YAML type is: "
                    "empty. The root YAML object must be a map or an array.");
}

TEST_F(ParseCommandDescriptor, InvalidJsonToDescriptor) {
  const std::filesystem::path descriptor_path(
      "test/fx/parser/__data__/invalid_descriptor.command.fx.yaml");

  expect_parse_fail(
      descriptor_path,
      "Invalid descriptor: "
      "test/fx/parser/__data__/invalid_descriptor.command.fx.yaml. "
      "INVALID_ARGUMENT:invalid_field: Cannot find field.");
}

TEST_F(ParseCommandDescriptor, ValidationFail) {
  const std::filesystem::path descriptor_path(
      "test/fx/parser/__data__/validation_fail.command.fx.yaml");

  expect_parse_fail(
      descriptor_path,
      "Invalid descriptor: "
      "test/fx/parser/__data__/validation_fail.command.fx.yaml. Unsupported "
      "descriptor "
      "version \"fake-version\", only v1beta is supported. Command synopsis "
      "cannot "
      "be empty. Runtime run cannot be empty.");
}

TEST_F(ParseCommandDescriptor, ValidYaml) {
  const std::filesystem::path descriptor_path(
      "test/fx/parser/__data__/valid.command.fx.yaml");

  fx::descriptor::v1beta::FxCommandDescriptor expected;
  expected.set_descriptor_version("v1beta");
  expected.set_synopsis("test");
  expected.mutable_runtime()->set_run("test-run");

  expect_parse_eq(descriptor_path, expected);
}

// JsonToDescriptor ------------------------------------------------------------

struct JsonToDescriptor : testing::Test {
  template <typename D>
  void static expect_convert_eq(const nlohmann::json& json, const D& expected) {
    const auto result = fx::parser::json_to_descriptor<D>(json);
    ASSERT_TRUE(result.ok()) << result.error();
    ASSERT_TRUE(google::protobuf::util::MessageDifferencer::Equals(
        expected, result.value()));
  }

  template <typename D>
  void static expect_convert_fail(const nlohmann::json& json,
                                  const std::string& expected_error) {
    const auto result = fx::parser::json_to_descriptor<D>(json);
    ASSERT_TRUE(result.failed());
    EXPECT_EQ(expected_error, result.error());
  };
};

TEST_F(JsonToDescriptor, EmptyJsonWorkspaceDescriptor) {
  const auto json = nlohmann::json::object();
  const fx::descriptor::v1beta::FxWorkspaceDescriptor expected;

  expect_convert_eq<fx::descriptor::v1beta::FxWorkspaceDescriptor>(json,
                                                                   expected);
};

TEST_F(JsonToDescriptor, InvalidJsonWorkspaceDescriptor) {
  auto json = nlohmann::json::object();
  json["_fake_field"] = true;

  expect_convert_fail<fx::descriptor::v1beta::FxWorkspaceDescriptor>(
      json, "INVALID_ARGUMENT:_fake_field: Cannot find field.");
};

TEST_F(JsonToDescriptor, ValidJsonWorkspaceDescriptor) {
  auto json = R"({
    "descriptor_version": "v1beta",
    "ignore": ["test", "node_modules", "bazel-*"]
  })"_json;
  fx::descriptor::v1beta::FxWorkspaceDescriptor expected;
  expected.set_descriptor_version("v1beta");
  expected.add_ignore("test");
  expected.add_ignore("node_modules");
  expected.add_ignore("bazel-*");

  expect_convert_eq<fx::descriptor::v1beta::FxWorkspaceDescriptor>(json,
                                                                   expected);
};

TEST_F(JsonToDescriptor, EmptyJsonCommandDescriptor) {
  const auto json = nlohmann::json::object();
  const fx::descriptor::v1beta::FxCommandDescriptor expected;

  expect_convert_eq<fx::descriptor::v1beta::FxCommandDescriptor>(json,
                                                                 expected);
};

TEST_F(JsonToDescriptor, InvalidJsonCommandDescriptor) {
  auto json = nlohmann::json::object();
  json["_fake_field"] = true;

  expect_convert_fail<fx::descriptor::v1beta::FxCommandDescriptor>(
      json, "INVALID_ARGUMENT:_fake_field: Cannot find field.");
};

TEST_F(JsonToDescriptor, ValidJsonCommandDescriptor) {
  auto json = R"({
    "descriptor_version": "v1beta",
    "synopsis": "example synopsis",
    "options": [
      {
        "name": "example",
        "short_name": "e",
        "description": "example description",
        "bool_value": {}
      }
    ]
  })"_json;
  fx::descriptor::v1beta::FxCommandDescriptor expected;
  expected.set_descriptor_version("v1beta");
  expected.set_synopsis("example synopsis");

  fx::descriptor::v1beta::OptionDescriptor option_one;
  option_one.set_name("example");
  option_one.set_short_name("e");
  option_one.set_description("example description");
  option_one.mutable_bool_value();
  expected.add_options()->CopyFrom(option_one);

  expect_convert_eq<fx::descriptor::v1beta::FxCommandDescriptor>(json,
                                                                 expected);
};
