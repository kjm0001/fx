#include "fx/argparse/argparse.hpp"
#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "fx/descriptor/v1beta/descriptor.pb.h"
#include "test/helper/helper.hpp"

struct Parse : testing::Test {
  void static expect_parse_eq(
      const fx::descriptor::v1beta::FxCommandDescriptor &descriptor,
      const std::vector<std::string> &arguments,
      const nlohmann::json &expected) {
    const auto result = fx::argparse::parse(arguments, descriptor);
    ASSERT_TRUE(result.ok()) << result.error();
    EXPECT_EQ(expected, result.value());
  }

  void static expect_parse_fail(
      const fx::descriptor::v1beta::FxCommandDescriptor &descriptor,
      const std::vector<std::string> &arguments,
      const std::string &expected_messge) {
    const auto result = fx::argparse::parse(arguments, descriptor);
    ASSERT_TRUE(result.failed());
    EXPECT_EQ(expected_messge, result.error());
  }
};

TEST_F(Parse, HelpFlag) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "bool-test", "bool_value": {}},
        {"name": "string-test", "string_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--help"};

  // user_set is intentionally false — this is wrong, however with a help flag
  // set, it should never reach the underlying command.
  expect_parse_eq(descriptor, arguments, R"({
    "bool-test": {"user_set": false, "value": false},
    "string-test": {"user_set": false, "value": ""},
    "help": {"user_set": false, "value": true}
  })"_json);
};

TEST_F(Parse, HelpShortFlag) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "bool-test", "bool_value": {}},
        {"name": "string-test", "string_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"-h"};

  // user_set is intentionally false — this is wrong, however with a help flag
  // set, it should never reach the underlying command.
  expect_parse_eq(descriptor, arguments, R"({
    "bool-test": {"user_set": false, "value": false},
    "string-test": {"user_set": false, "value": ""},
    "help": {"user_set": false, "value": true}
  })"_json);
};

TEST_F(Parse, DefaultOptionsWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "bool-test", "bool_value": {}},
        {"name": "string-test", "string_value": {}},
        {"name": "string-list-test", "string_value": {"list": true}},
        {"name": "int-test", "int_value": {}},
        {"name": "int-list-test", "int_value": {"list": true}},
        {"name": "double-test", "double_value": {}},
        {"name": "double-list-test", "double_value": {"list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "bool-test": {"user_set": false, "value": false},
    "string-test": {"user_set": false, "value": ""},
    "string-list-test": {"user_set": false, "value": [""]},
    "int-test": {"user_set": false, "value": 0},
    "int-list-test": {"user_set": false, "value": [0]},
    "double-test": {"user_set": false, "value": 0.0},
    "double-list-test": {"user_set": false, "value": [0.0]},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, DefaultScalarArgumentsWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {}},
        {"name": "int-test", "int_value": {}},
        {"name": "double-test", "double_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": false, "value": ""},
    "int-test": {"user_set": false, "value": 0},
    "double-test": {"user_set": false, "value": 0.0},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, DefaultScalarArgumentsWithDoubleListArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {}},
        {"name": "int-test", "int_value": {}},
        {"name": "double-test", "double_value": {"list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": false, "value": ""},
    "int-test": {"user_set": false, "value": 0},
    "double-test": {"user_set": false, "value": [0.0]},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, DefaultScalarArgumentsWithIntListArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {}},
        {"name": "double-test", "double_value": {}},
        {"name": "int-test", "int_value": {"list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": false, "value": ""},
    "int-test": {"user_set": false, "value": [0]},
    "double-test": {"user_set": false, "value": 0.0},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, DefaultScalarArgumentsWithStringListArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {}},
        {"name": "int-test", "int_value": {}},
        {"name": "string-test", "string_value": {"list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": false, "value": [""]},
    "int-test": {"user_set": false, "value": 0},
    "double-test": {"user_set": false, "value": 0.0},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, CustomDefaultOptionsWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "bool-test", "bool_value": {}},
        {"name": "string-test", "string_value": {"default": "custom"}},
        {"name": "string-list-test", "string_value": {"list": true, "default": "custom-list"}},
        {"name": "int-test", "int_value": {"default": 416}},
        {"name": "int-list-test", "int_value": {"list": true, "default": -905}},
        {"name": "double-test", "double_value": {"default": 90.5}},
        {"name": "double-list-test", "double_value": {"list": true, "default": -9.5}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "bool-test": {"user_set": false, "value": false},
    "string-test": {"user_set": false, "value": "custom"},
    "string-list-test": {"user_set": false, "value": ["custom-list"]},
    "int-test": {"user_set": false, "value": 416},
    "int-list-test": {"user_set": false, "value": [-905]},
    "double-test": {"user_set": false, "value": 90.5},
    "double-list-test": {"user_set": false, "value": [-9.5]},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, CustomDefaultScalarArgumentsWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"default": "custom"}},
        {"name": "int-test", "int_value": {"default": 416}},
        {"name": "double-test", "double_value": {"default": 90.5}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": false, "value": "custom"},
    "int-test": {"user_set": false, "value": 416},
    "double-test": {"user_set": false, "value": 90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse,
       CustomDefaultScalarArgumentsWithDoubleListArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"default": "custom"}},
        {"name": "int-test", "int_value": {"default": 416}},
        {"name": "double-test", "double_value": {"list": true, "default": 90.5}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": false, "value": "custom"},
    "int-test": {"user_set": false, "value": 416},
    "double-test": {"user_set": false, "value": [90.5]},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, CustomDefaultScalarArgumentsWithIntListArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"default": "custom"}},
        {"name": "double-test", "double_value": {"default": 90.5}},
        {"name": "int-test", "int_value": {"list": true, "default": 416}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": false, "value": "custom"},
    "int-test": {"user_set": false, "value": [416]},
    "double-test": {"user_set": false, "value": 90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse,
       CustomDefaultScalarArgumentsWithStringListArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {"default": 90.5}},
        {"name": "int-test", "int_value": {"default": 416}},
        {"name": "string-test", "string_value": {"list": true, "default": "custom"}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": false, "value": ["custom"]},
    "int-test": {"user_set": false, "value": 416},
    "double-test": {"user_set": false, "value": 90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, DefaultOptionsWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "bool-test", "bool_value": {}},
        {"name": "string-test", "string_value": {}},
        {"name": "string-list-test", "string_value": {"list": true}},
        {"name": "int-test", "int_value": {}},
        {"name": "int-list-test", "int_value": {"list": true}},
        {"name": "double-test", "double_value": {}},
        {"name": "double-list-test", "double_value": {"list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--bool-test",
                                           // -
                                           "--string-test", "hello",
                                           // -
                                           "--string-list-test", "goodbye",
                                           // -
                                           "--string-list-test", "friend",
                                           // -
                                           "--int-test", "416",
                                           // -
                                           "--int-list-test", "90",
                                           // -
                                           "--int-list-test", "5",
                                           // -
                                           "--double-test", "-90.5",
                                           // -
                                           "--double-list-test", "-0.4",
                                           // -
                                           "--double-list-test", "1.6"};

  expect_parse_eq(descriptor, arguments, R"({
    "bool-test": {"user_set": true, "value": true},
    "string-test": {"user_set": true, "value": "hello"},
    "string-list-test": {"user_set": true, "value": ["goodbye", "friend"]},
    "int-test": {"user_set": true, "value": 416},
    "int-list-test": {"user_set": true, "value": [90, 5]},
    "double-test": {"user_set": true, "value": -90.5},
    "double-list-test": {"user_set": true, "value": [-0.4, 1.6]},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, DefaultScalarArgumentsWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {}},
        {"name": "int-test", "int_value": {}},
        {"name": "double-test", "double_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"hello", "416", "90.5"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "int-test": {"user_set": true, "value": 416},
    "double-test": {"user_set": true, "value": 90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, DefaultScalarArgumentsWithDoubleListArgumentWithoUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {}},
        {"name": "int-test", "int_value": {}},
        {"name": "double-test", "double_value": {"list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"hello", "416", "90.5", "103.5",
                                           "93.5"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "int-test": {"user_set": true, "value": 416},
    "double-test": {"user_set": true, "value": [90.5, 103.5, 93.5]},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, DefaultScalarArgumentsWithIntListArgumentWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {}},
        {"name": "double-test", "double_value": {}},
        {"name": "int-test", "int_value": {"list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"hello", "90.5", "416", "415",
                                           "-647"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "int-test": {"user_set": true, "value": [416, 415, -647]},
    "double-test": {"user_set": true, "value": 90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, DefaultScalarArgumentsWithStringListArgumentWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {}},
        {"name": "int-test", "int_value": {}},
        {"name": "string-test", "string_value": {"list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"90.5", "416", "hello", "world",
                                           "test"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": ["hello", "world", "test"]},
    "int-test": {"user_set": true, "value": 416},
    "double-test": {"user_set": true, "value": 90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, CustomDefaultOptionsWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "bool-test", "bool_value": {}},
        {"name": "string-test", "string_value": {"default": "custom"}},
        {"name": "string-list-test", "string_value": {"list": true, "default": "custom-list"}},
        {"name": "int-test", "int_value": {"default": 416}},
        {"name": "int-list-test", "int_value": {"list": true, "default": -905}},
        {"name": "double-test", "double_value": {"default": 90.5}},
        {"name": "double-list-test", "double_value": {"list": true, "default": -9.5}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--bool-test",
                                           // -
                                           "--string-test", "hello",
                                           // -
                                           "--string-list-test", "goodbye",
                                           // -
                                           "--string-list-test", "friend",
                                           // -
                                           "--int-test", "416",
                                           // -
                                           "--int-list-test", "90",
                                           // -
                                           "--int-list-test", "5",
                                           // -
                                           "--double-test", "-90.5",
                                           // -
                                           "--double-list-test", "-0.4",
                                           // -
                                           "--double-list-test", "1.6"};

  expect_parse_eq(descriptor, arguments, R"({
    "bool-test": {"user_set": true, "value": true},
    "string-test": {"user_set": true, "value": "hello"},
    "string-list-test": {"user_set": true, "value": ["goodbye", "friend"]},
    "int-test": {"user_set": true, "value": 416},
    "int-list-test": {"user_set": true, "value": [90, 5]},
    "double-test": {"user_set": true, "value": -90.5},
    "double-list-test": {"user_set": true, "value": [-0.4, 1.6]},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, CustomDefaultScalarArgumentsWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"default": "custom"}},
        {"name": "int-test", "int_value": {"default": 416}},
        {"name": "double-test", "double_value": {"default": 90.5}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"hello", "416", "90.5"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "int-test": {"user_set": true, "value": 416},
    "double-test": {"user_set": true, "value": 90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, CustomDefaultScalarArgumentsWithDoubleListArgumentWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"default": "custom"}},
        {"name": "int-test", "int_value": {"default": 416}},
        {"name": "double-test", "double_value": {"list": true, "default": 90.5}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"hello", "416", "103.5", "93.5"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "int-test": {"user_set": true, "value": 416},
    "double-test": {"user_set": true, "value": [103.5, 93.5]},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, CustomDefaultScalarArgumentsWithIntListArgumentWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"default": "custom"}},
        {"name": "double-test", "double_value": {"default": 90.5}},
        {"name": "int-test", "int_value": {"list": true, "default": 416}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"hello", "90.5", "415", "-647"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "int-test": {"user_set": true, "value": [415, -647]},
    "double-test": {"user_set": true, "value": 90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, CustomDefaultScalarArgumentsWithStringListArgumentWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {"default": 90.5}},
        {"name": "int-test", "int_value": {"default": 416}},
        {"name": "string-test", "string_value": {"list": true, "default": "custom"}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"90.5", "416", "world", "test"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": ["world", "test"]},
    "int-test": {"user_set": true, "value": 416},
    "double-test": {"user_set": true, "value": 90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, OverloadScalarIntOptionType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "int-test", "int_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--int-test", "416", "--int-test",
                                           "905"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unrecognized token: --int-test");
};

TEST_F(Parse, OverloadScalarDoubleOptionType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "double-test", "double_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--double-test", "4.16",
                                           "--double-test", "90.5"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unrecognized token: --double-test");
};

TEST_F(Parse, OverloadScalarStringOptionType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "string-test", "string_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--string-test", "hello",
                                           "--string-test", "world"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unrecognized token: --string-test");
};

TEST_F(Parse, OverloadScalarIntArgumentType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "int-test", "int_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"416", "905"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unrecognized token: 905");
};

TEST_F(Parse, OverloadScalarDoubleArgumentType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"4.16", "90.5"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unrecognized token: 90.5");
};

TEST_F(Parse, OverloadScalarStringArgumentType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"hello", "world"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unrecognized token: world");
};

TEST_F(Parse, InvalidIntOptionType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "int-test", "int_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--int-test", "test"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unable to convert 'test' to destination type");
};

TEST_F(Parse, InvalidDoubleOptionType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "double-test", "int_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--double-test", "test"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unable to convert 'test' to destination type");
};

TEST_F(Parse, InvalidIntArgumentType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "int-test", "int_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"test"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unable to convert 'test' to destination type");
};

TEST_F(Parse, InvalidDoubleArgumentType) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "int_value": {}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"test"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Unable to convert 'test' to destination type");
};

TEST_F(Parse, RequiredIntOptionWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "int-test", "int_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: --int-test <int-test>");
};

TEST_F(Parse, RequiredDoubleOptionWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "double-test", "double_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: --double-test <double-test>");
};

TEST_F(Parse, RequiredStringOptionWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "string-test", "string_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: --string-test <string-test>");
};

TEST_F(Parse, RequiredIntOptionWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "int-test", "int_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--int-test", "416"};

  expect_parse_eq(descriptor, arguments, R"({
    "int-test": {"user_set": true, "value": 416},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, RequiredDoubleOptionWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "double-test", "double_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--double-test", "-90.5"};

  expect_parse_eq(descriptor, arguments, R"({
    "double-test": {"user_set": true, "value": -90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, RequiredStringOptionWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "string-test", "string_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--string-test", "hello"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, RequiredIntListOptionWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "int-test", "int_value": {"required": true, "list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: --int-test <int-test>");
};

TEST_F(Parse, RequiredDoubleListOptionWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "double-test", "double_value": {"required": true, "list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: --double-test <double-test>");
};

TEST_F(Parse, RequiredStringListOptionWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "string-test", "string_value": {"required": true, "list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: --string-test <string-test>");
};

TEST_F(Parse, RequiredIntArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "int-test", "int_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments, "[argparse] Expected: <int-test>");
};

TEST_F(Parse, RequiredDoubleArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: <double-test>");
};

TEST_F(Parse, RequiredStringArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: <string-test>");
};

TEST_F(Parse, RequiredIntArgumentWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "int-test", "int_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"416"};

  expect_parse_eq(descriptor, arguments, R"({
    "int-test": {"user_set": true, "value": 416},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, RequiredDoubleArgumentWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"-90.5"};

  expect_parse_eq(descriptor, arguments, R"({
    "double-test": {"user_set": true, "value": -90.5},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, RequiredStringArgumentWithUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"required": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"hello"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, RequiredIntListArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "int-test", "int_value": {"required": true, "list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: <int-test> [<int-test>...]");
};

TEST_F(Parse, RequiredDoubleListArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {"required": true, "list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: <double-test> [<double-test>...]");
};

TEST_F(Parse, RequiredStringListArgumentWithoutUserInput) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"required": true, "list": true}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Expected: <string-test> [<string-test>...]");
};

TEST_F(Parse, InvalidIntOptionChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "int-test", "int_value": {"choices": [4, 1, 6]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--int-test", "905"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Value '905' not expected.");
};

TEST_F(Parse, InvalidDoubleOptionChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "double-test", "double_value": {"choices": [4.1, 1.2, 6.3]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--double-test", "905"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Value '905' not expected.");
};

TEST_F(Parse, InvalidStringOptionChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "string-test", "string_value": {"choices": ["hello", "world"]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--string-test", "905"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Value '905' not expected.");
};

TEST_F(Parse, ValidIntOptionChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "int-test", "int_value": {"choices": [4, 1, 6]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--int-test", "4"};

  expect_parse_eq(descriptor, arguments, R"({
    "int-test": {"user_set": true, "value": 4},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, ValidDoubleOptionChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "double-test", "double_value": {"choices": [4.1, 1.2, 6.3]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--double-test", "6.3"};

  expect_parse_eq(descriptor, arguments, R"({
    "double-test": {"user_set": true, "value": 6.3},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, ValidStringOptionChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "string-test", "string_value": {"choices": ["hello", "world"]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"--string-test", "hello"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, InvalidIntArgumentChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "int-test", "int_value": {"choices": [4, 1, 6]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"905"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Value '905' not expected.");
};

TEST_F(Parse, InvalidDoubleArgumentChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {"choices": [4.1, 1.2, 6.3]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"905"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Value '905' not expected.");
};

TEST_F(Parse, InvalidStringArgumentChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"choices": ["hello", "world"]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"905"};

  expect_parse_fail(descriptor, arguments,
                    "[argparse] Value '905' not expected.");
};

TEST_F(Parse, ValidIntArgumentChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "int-test", "int_value": {"choices": [4, 1, 6]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"4"};

  expect_parse_eq(descriptor, arguments, R"({
    "int-test": {"user_set": true, "value": 4},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, ValidDoubleArgumentChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "double-test", "double_value": {"choices": [4.1, 1.2, 6.3]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"6.3"};

  expect_parse_eq(descriptor, arguments, R"({
    "double-test": {"user_set": true, "value": 6.3},
    "help": {"user_set": false,"value": false}
  })"_json);
};

TEST_F(Parse, ValidStringArgumentChoice) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "string-test", "string_value": {"choices": ["hello", "world"]}}
      ]
    }
  )"_json);

  const std::vector<std::string> arguments{"hello"};

  expect_parse_eq(descriptor, arguments, R"({
    "string-test": {"user_set": true, "value": "hello"},
    "help": {"user_set": false,"value": false}
  })"_json);
};
