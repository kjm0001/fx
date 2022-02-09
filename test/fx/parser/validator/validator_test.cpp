#include "fx/parser/validator/validator.hpp"
#include <fmt/core.h>
#include <fmt/ranges.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "fx/descriptor/v1beta/descriptor.pb.h"
#include "fx/result/result.hpp"
#include "test/helper/helper.hpp"

// ValidateDescriptor ----------------------------------------------------------

struct ValidateDescriptor : testing::Test {
  template <typename D>
  void static expect_validate_ok(D& descriptor) {
    auto result = fx::parser::validator::validate_descriptor(descriptor);
    ASSERT_TRUE(result.ok()) << result.error();
  }

  template <typename D>
  void static expect_validate_errors(D& descriptor,
                                     const std::string& expected_error) {
    auto result = fx::parser::validator::validate_descriptor(descriptor);
    ASSERT_TRUE(result.failed());
    EXPECT_EQ(expected_error, result.error());
  }
};

TEST_F(ValidateDescriptor, ValidWorkspace) {
  const auto descriptor = fx::test::helper::workspace_descriptor(R"(
    {"descriptor_version": "v1beta"}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateDescriptor, InvalidWorkspace) {
  const auto descriptor = fx::test::helper::workspace_descriptor(R"(
    {"descriptor_version": "fake"}
  )"_json);

  const std::string expected_errors =
      "Unsupported descriptor version \"fake\", only v1beta is supported.";

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateDescriptor, ValidCommand) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "descriptor_version": "v1beta",
      "synopsis": "test",
      "options": [
        {
          "name": "opt1",
          "short_name": "o",
          "description": "test opt",
          "bool_value": {}
        }
      ],
      "arguments": [
        {
          "name": "arg1",
          "description": "test arg",
          "int_value": {
            "choices": [4, 1, 6],
            "default": 4,
            "required": true,
            "list": true
          }
        }
      ],
      "runtime": {
        "run": "test-run"
      }
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateDescriptor, InvalidCommand) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "descriptor_version": "fake",
      "options": [
        {"name": "o", "short_name": "long"}
      ],
      "arguments": [
        {
          "name": "arg1",
          "description": "test arg",
          "int_value": {
            "choices": [4, 1, 6],
            "default": 905,
            "list": true
          }
        },
        {
          "name": "arg2",
          "description": "test arg",
          "int_value": {
            "required": true
          }
        }
      ]
    }
  )"_json);

  const std::string expected_errors =
      "Unsupported descriptor version \"fake\", only v1beta is supported. "
      "Command "
      "synopsis cannot be empty. Runtime run cannot be empty. Option[index:0] "
      "\"o\" name cannot be shorter than two characters. Option[index:0] "
      "\"long\" short name cannot be longer than one character. "
      "Option[index:0] \"o\" description cannot be empty. Option[index:0] "
      "\"o\" value cannot be empty. Argument[index:0] \"arg1\" default value "
      "\"905\" is invalid, the default value must be one of the choices: [4, "
      "1, 6]. To prevent ambiguous argument parsing, only the last argument "
      "can be a list. Arguments at index [0] need to also be required, because "
      "arguments preceding a required argument must be requried to prevent "
      "ambiguous parsing.";

  expect_validate_errors(descriptor, expected_errors);
}

// ValidateCommand -------------------------------------------------------------

struct ValidateCommand : testing::Test {
  void static expect_validate_ok(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_command(descriptor, error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  void static expect_validate_errors(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      std::vector<std::string>& expected_errors) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_command(descriptor, error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    EXPECT_EQ(expected_errors, error_messages);
  }
};

TEST_F(ValidateCommand, Valid) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {"descriptor_version": "v1beta", "synopsis": "test", "runtime": {"run": "run-test"}}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateCommand, UnsupportedVersion) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {"descriptor_version": "fake", "synopsis": "test", "runtime": {"run": "run-test"}}
  )"_json);

  std::vector<std::string> expected_errors{
      "Unsupported descriptor version \"fake\", only v1beta is supported."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateCommand, EmptySynopsis) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {"descriptor_version": "v1beta", "runtime": {"run": "run-test"}}
  )"_json);

  std::vector<std::string> expected_errors{"Command synopsis cannot be empty."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateCommand, EmptyRuntime) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {"descriptor_version": "v1beta", "synopsis": "test"}
  )"_json);

  std::vector<std::string> expected_errors{"Runtime run cannot be empty."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateCommand, Empty) {
  const auto descriptor = fx::test::helper::command_descriptor(R"({})"_json);

  std::vector<std::string> expected_errors{
      "Unsupported descriptor version \"\", only v1beta is supported.",
      "Command synopsis cannot be empty.", "Runtime run cannot be empty."};

  expect_validate_errors(descriptor, expected_errors);
}

// ValidateOptions -------------------------------------------------------------

struct ValidateOptions : testing::Test {
  void static expect_validate_ok(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_options(descriptor.options(),
                                            error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  void static expect_validate_errors(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      std::vector<std::string>& expected_errors) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_options(descriptor.options(),
                                            error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    EXPECT_EQ(expected_errors, error_messages);
  }
};

TEST_F(ValidateOptions, ValidatesValid) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "opt1", "description": "test", "bool_value": {}},
        {"name": "opt2", "short_name": "a", "description": "test", "int_value": {}},
        {"name": "opt3", "short_name": "b", "description": "test", "double_value": {}},
        {"name": "opt4", "short_name": "c", "description": "test", "string_value": {}}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateOptions, PropgateError) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "o", "description": "test", "bool_value": {}},
        {"name": "opt2", "short_name": "long", "description": "test", "int_value": {}},
        {"name": "opt3"},
        {"name": "opt4", "description": "test", "int_value": {"choices": [4, 1, 6], "default": 905}}
      ]
    }
  )"_json);

  std::vector<std::string> expected_errors{};
  expected_errors.emplace_back(
      "Option[index:0] \"o\" name cannot be shorter than two characters.");
  expected_errors.emplace_back(
      "Option[index:1] \"long\" short name cannot be longer than one "
      "character.");
  expected_errors.emplace_back(
      "Option[index:2] \"opt3\" description cannot be empty.");
  expected_errors.emplace_back(
      "Option[index:2] \"opt3\" value cannot be empty.");
  expected_errors.emplace_back(
      "Option[index:3] \"opt4\" default value \"905\" is invalid, the default "
      "value must be one of the choices: [4, 1, 6].");

  expect_validate_errors(descriptor, expected_errors);
}

// ValidateArguments -----------------------------------------------------------

struct ValidateArguments : testing::Test {
  void static expect_validate_ok(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_arguments(descriptor.arguments(),
                                              error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  void static expect_validate_errors(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      std::vector<std::string>& expected_errors) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_arguments(descriptor.arguments(),
                                              error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    EXPECT_EQ(expected_errors, error_messages);
  }
};

TEST_F(ValidateArguments, ValidatesValid) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "arg1", "description": "test", "int_value": {}},
        {"name": "arg2", "description": "test", "double_value": {}},
        {"name": "arg3", "description": "test", "string_value": {}}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateArguments, PropgateError) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "description": "test", "int_value": {"list": true}},
        {"name": "arg2", "description": "", "double_value": {"required": true}},
        {"name": "arg3", "description": "test"},
        {"name": "arg4", "description": "test", "int_value": {"choices": [4,1,6], "default": 905}}
      ]
    }
  )"_json);

  std::vector<std::string> expected_errors{};

  expected_errors.emplace_back(
      "Argument[index:0] \"a\" name cannot be shorter than two characters.");
  expected_errors.emplace_back(
      "Argument[index:1] \"arg2\" description cannot be empty.");
  expected_errors.emplace_back(
      "Argument[index:2] \"arg3\" value cannot be empty.");
  expected_errors.emplace_back(
      "Argument[index:3] \"arg4\" default value \"905\" is invalid, the "
      "default value must be one of the choices: [4, 1, 6].");
  expected_errors.emplace_back(
      "To prevent ambiguous argument parsing, only the last argument can be a "
      "list.");
  expected_errors.emplace_back(
      "Arguments at index [0] need to also be required, because arguments "
      "preceding a "
      "required argument must be requried to prevent ambiguous parsing.");

  expect_validate_errors(descriptor, expected_errors);
}

// ValidateName ----------------------------------------------------------------

struct ValidateName : testing::Test {
  template <typename D>
  void static expect_validate_ok(const D& descriptor) {
    const std::string error_prefix;
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_name(descriptor, error_prefix,
                                         error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  template <typename D>
  void static expect_validate_errors(
      const D& descriptor, std::vector<std::string>& expected_errors) {
    const std::string error_prefix = "[test-prefix]";
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_name(descriptor, error_prefix,
                                         error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    EXPECT_EQ(expected_errors, error_messages);
  }
};

TEST_F(ValidateName, OptionValidName) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "hello"}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateName, OptionShortName) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "x"}
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"x\" name cannot be shorter than two characters."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateName, OptionReservedLowerhelp) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "help"}
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] name cannot be named \"help\" (reserved)."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateName, OptionReservedUpperHELP) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "HELP"}
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] name cannot be named \"help\" (reserved)."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateName, ArgumentValidName) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {"name": "hello"}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateName, ArgumentShortName) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {"name": "x"}
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"x\" name cannot be shorter than two characters."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateName, ArgumentReservedLowerhelp) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {"name": "help"}
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] name cannot be named \"help\" (reserved)."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateName, ArgumentReservedUpperHELP) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {"name": "HELP"}
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] name cannot be named \"help\" (reserved)."};

  expect_validate_errors(descriptor, expected_errors);
}

// ValidateShortName -----------------------------------------------------------

struct ValidateShortName : testing::Test {
  void static expect_validate_ok(
      const fx::descriptor::v1beta::OptionDescriptor& descriptor) {
    const std::string error_prefix;
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_short_name(descriptor, error_prefix,
                                               error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  void static expect_validate_errors(
      const fx::descriptor::v1beta::OptionDescriptor& descriptor,
      std::vector<std::string>& expected_errors) {
    const std::string error_prefix = "[test-prefix]";
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_short_name(descriptor, error_prefix,
                                               error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    EXPECT_EQ(expected_errors, error_messages);
  }
};

TEST_F(ValidateShortName, ValidName) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"short_name": "x"}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateShortName, LongName) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"short_name": "longname"}
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"longname\" short name cannot be longer than one "
      "character."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateShortName, ReservedLowerh) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "test", "short_name": "h"}
  )"_json);

  std::vector<std::string> expected_errors{
      R"([test-prefix] "test" short name cannot be "h" (reserved).)"};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateShortName, ReservedUpperH) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "test", "short_name": "H"}
  )"_json);

  std::vector<std::string> expected_errors{
      R"([test-prefix] "test" short name cannot be "h" (reserved).)"};

  expect_validate_errors(descriptor, expected_errors);
}

// ValidateDescription ---------------------------------------------------------

struct ValidateDescription : testing::Test {
  template <typename D>
  void static expect_validate_ok(const D& descriptor) {
    const std::string error_prefix;
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_description(descriptor, error_prefix,
                                                error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  template <typename D>
  void static expect_validate_errors(
      const D& descriptor, std::vector<std::string>& expected_errors) {
    const std::string error_prefix = "[test-prefix]";
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_description(descriptor, error_prefix,
                                                error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    EXPECT_EQ(expected_errors, error_messages);
  }
};

TEST_F(ValidateDescription, OptionValidDescription) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"description": "something, anything!"}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateDescription, ArgumentValidDescription) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {"description": "something, anything!"}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateDescription, OptionEmptyDescription) {
  const auto descriptor = fx::test::helper::option_descriptor(R"({
    "name": "test"
  })"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" description cannot be empty."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateDescription, ArgumentEmptyDescription) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"({
    "name": "test"
  })"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" description cannot be empty."};

  expect_validate_errors(descriptor, expected_errors);
}

// ValidateValue ---------------------------------------------------------------

struct ValidateValue : testing::Test {
  template <typename D>
  void static expect_validate_ok(const D& descriptor) {
    const std::string error_prefix;
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_value(descriptor, error_prefix,
                                          error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  template <typename D>
  void static expect_validate_errors(
      const D& descriptor, std::vector<std::string>& expected_errors) {
    const std::string error_prefix = "[test-prefix]";
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_value(descriptor, error_prefix,
                                          error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    EXPECT_EQ(expected_errors, error_messages);
  }
};

TEST_F(ValidateValue, OptionWithoutIntChoices) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "test", "int_value": {"default": 416}}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, OptionWithoutDoubleChoices) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "test", "double_value": {"default": 41.6}}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, OptionWithoutStringChoices) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "test", "string_value": {"default": "ok"}}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, ArgumentWithoutIntChoices) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {"name": "test", "int_value": {"default": 416}}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, ArgumentWithoutDoubleChoices) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {"name": "test", "double_value": {"default": 41.6}}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, ArgumentWithoutStringChoices) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {"name": "test", "string_value": {"default": "ok"}}
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, OptionWithValidIntDefault) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {
      "name": "test",
      "int_value": {
        "choices": [4, 1, 6],
        "default": 6
      }
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, OptionWithValidDoubleDefault) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {
      "name": "test",
      "double_value": {
        "choices": [4.1, 1.2, 6.3],
        "default": 4.1
      }
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, OptionWithValidStringDefault) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {
      "name": "test",
      "string_value": {
        "choices": ["a", "b", "c"],
        "default": "c"
      }
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, ArgumentWithValidIntDefault) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {
      "name": "test",
      "int_value": {
        "choices": [4, 1, 6],
        "default": 6
      }
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, ArgumentWithValidDoubleDefault) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {
      "name": "test",
      "double_value": {
        "choices": [4.1, 1.2, 6.3],
        "default": 4.1
      }
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, ArgumentWithValidStringDefault) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {
      "name": "test",
      "string_value": {
        "choices": ["a", "b", "c"],
        "default": "c"
      }
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateValue, EmptyOptionValue) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {"name": "test"}
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" value cannot be empty."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateValue, EmptyArgumentValue) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {"name": "test"}
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" value cannot be empty."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateValue, OptionWithInvalidIntDefault) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {
      "name": "test",
      "int_value": {
        "choices": [4, 1, 6],
        "default": -905
      }
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" default value \"-905\" is invalid, the default "
      "value must be one of the choices: [4, 1, 6]."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateValue, OptionWithInvalidDoubleDefault) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {
      "name": "test",
      "double_value": {
        "choices": [4.1, 1.2, 6.3],
        "default": -90.5
      }
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" default value \"-90.5\" is invalid, the default "
      "value must be one of the choices: [4.1, 1.2, 6.3]."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateValue, OptionWithInvalidStringDefault) {
  const auto descriptor = fx::test::helper::option_descriptor(R"(
    {
      "name": "test",
      "string_value": {
        "choices": ["a", "b", "c"],
        "default": "fail"
      }
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" default value \"fail\" is invalid, the default "
      "value must be one of the choices: [a, b, c]."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateValue, ArgumentWithInvalidIntDefault) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {
      "name": "test",
      "int_value": {
        "choices": [4, 1, 6],
        "default": -905
      }
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" default value \"-905\" is invalid, the default "
      "value must be one of the choices: [4, 1, 6]."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateValue, ArgumentWithInvalidDoubleDefault) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {
      "name": "test",
      "double_value": {
        "choices": [4.1, 1.2, 6.3],
        "default": -90.5
      }
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" default value \"-90.5\" is invalid, the default "
      "value must be one of the choices: [4.1, 1.2, 6.3]."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateValue, ArgumentWithInvalidStringDefault) {
  const auto descriptor = fx::test::helper::argument_descriptor(R"(
    {
      "name": "test",
      "string_value": {
        "choices": ["a", "b", "c"],
        "default": "fail"
      }
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "[test-prefix] \"test\" default value \"fail\" is invalid, the default "
      "value must be one of the choices: [a, b, c]."};

  expect_validate_errors(descriptor, expected_errors);
}

// ValidateArgumentListConstraint ----------------------------------------------

struct ValidateArgumentListConstraint : testing::Test {
  void static expect_validate_ok(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_argument_list_constraint(
        descriptor.arguments(), error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  void static expect_validate_errors(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      std::vector<std::string>& expected_errors) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_argument_list_constraint(
        descriptor.arguments(), error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    EXPECT_EQ(expected_errors, error_messages);
  }
};

TEST_F(ValidateArgumentListConstraint, Empty) {
  const auto descriptor = fx::test::helper::command_descriptor(R"({})"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateArgumentListConstraint, NoLists) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {}},
        {"name": "b", "int_value": {}},
        {"name": "c", "double_value": {}}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateArgumentListConstraint, SingleListArgument) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {"list": true}}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateArgumentListConstraint, FirstListRestNotList) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {"list": true}},
        {"name": "b", "int_value": {}},
        {"name": "c", "double_value": {}}
      ]
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "To prevent ambiguous argument parsing, only the last argument can be a "
      "list."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateArgumentListConstraint, LastListPreviousSomeList) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {}},
        {"name": "b", "int_value": {"list": true}},
        {"name": "c", "double_value": {}},
        {"name": "d", "double_value": {"list": true}}
      ]
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "To prevent ambiguous argument parsing, only the last argument can be a "
      "list."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateArgumentListConstraint, LastListPreviousNotList) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {}},
        {"name": "b", "int_value": {}},
        {"name": "c", "double_value": {}},
        {"name": "d", "double_value": {"list": true}}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

// ValidateUniqueNameConstraint ------------------------------------------------

struct ValidateUniqueNameConstraint : testing::Test {
  void static expect_validate_ok(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_unique_name_constraint(
        descriptor.options(), descriptor.arguments(), error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  void static expect_validate_errors(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      std::vector<std::string>& expected_errors) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_unique_name_constraint(
        descriptor.options(), descriptor.arguments(), error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    // Ignore order of error messages due to looping though unordered_map
    EXPECT_THAT(expected_errors,
                testing::UnorderedElementsAreArray(error_messages));
  }
};

TEST_F(ValidateUniqueNameConstraint, Valid) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "opt1", "short_name": "a"},
        {"name": "opt2", "short_name": "b"}
      ],
      "arguments": [
        {"name": "arg1"},
        {"name": "arg2"}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateUniqueNameConstraint, DuplicateNames) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "options": [
        {"name": "opt1", "short_name": "a"},
        {"name": "opt2", "short_name": "b"},
        {"name": "opt3", "short_name": "c"},
        {"name": "opt2", "short_name": "d"},
        {"name": "opt5", "short_name": "c"}
      ],
      "arguments": [
        {"name": "arg1"},
        {"name": "arg2"},
        {"name": "arg3"},
        {"name": "arg3"},
        {"name": "arg4"},
        {"name": "arg3"}
      ]
    }
  )"_json);

  std::vector<std::string> expected_errors{};
  expected_errors.emplace_back(
      "Name \"opt2\" used in Option[index:1], Option[index:3] is not unique.");
  expected_errors.emplace_back(
      "Name \"arg3\" used in Argument[index:2], Argument[index:3], "
      "Argument[index:5] is not unique.");
  expected_errors.emplace_back(
      "Short name \"c\" used in Option[index:2], Option[index:4] is not "
      "unique.");

  expect_validate_errors(descriptor, expected_errors);
}

// ValidateArgumentRequireConstraint -------------------------------------------

struct ValidateArgumentRequireConstraint : testing::Test {
  void static expect_validate_ok(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_argument_require_constraint(
        descriptor.arguments(), error_messages);
    EXPECT_TRUE(error_messages.empty())
        << fmt::format("{}", fmt::join(error_messages, " "));
  }

  void static expect_validate_errors(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      std::vector<std::string>& expected_errors) {
    std::vector<std::string> error_messages{};
    fx::parser::validator::validate_argument_require_constraint(
        descriptor.arguments(), error_messages);
    EXPECT_EQ(expected_errors.size(), error_messages.size());
    EXPECT_EQ(expected_errors, error_messages);
  }
};

TEST_F(ValidateArgumentRequireConstraint, Empty) {
  const auto descriptor = fx::test::helper::command_descriptor(R"({})"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateArgumentRequireConstraint, AllNotRequired) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {}},
        {"name": "b", "int_value": {}},
        {"name": "c", "double_value": {}}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateArgumentRequireConstraint, AllRequired) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {"required": true}},
        {"name": "b", "int_value": {"required": true}},
        {"name": "c", "double_value": {"required": true}}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateArgumentRequireConstraint, FirstNotRequiredRestRequired) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {}},
        {"name": "b", "string_value": {"required": true}},
        {"name": "c", "int_value": {"required": true}},
        {"name": "d", "double_value": {"required": true}}
      ]
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "Arguments at index [0] need to also be required, because arguments "
      "preceding a "
      "required argument must be requried to prevent ambiguous parsing."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateArgumentRequireConstraint, FirstRequiredRestNotRequired) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {"required": true}},
        {"name": "c", "int_value": {}},
        {"name": "d", "double_value": {}}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateArgumentRequireConstraint, LastNotRequiredPreviousAllRequired) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {"required": true}},
        {"name": "c", "int_value": {"required": true}},
        {"name": "d", "int_value": {"required": true}},
        {"name": "e", "int_value": {"required": true}},
        {"name": "f", "double_value": {}}
      ]
    }
  )"_json);

  expect_validate_ok(descriptor);
}

TEST_F(ValidateArgumentRequireConstraint, LastRequiredPreviousNotRequired) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {}},
        {"name": "c", "int_value": {}},
        {"name": "d", "int_value": {}},
        {"name": "e", "int_value": {}},
        {"name": "f", "double_value": {"required": true}}
      ]
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "Arguments at index [0, 1, 2, 3] need to also be required, because "
      "arguments "
      "preceding a required argument must be requried to prevent ambiguous "
      "parsing."};

  expect_validate_errors(descriptor, expected_errors);
}

TEST_F(ValidateArgumentRequireConstraint, MiddleNotRequired) {
  const auto descriptor = fx::test::helper::command_descriptor(R"(
    {
      "arguments": [
        {"name": "a", "string_value": {"required": true}},
        {"name": "c", "int_value": {}},
        {"name": "d", "int_value": {}},
        {"name": "e", "int_value": {"required": true}},
        {"name": "f", "int_value": {}},
        {"name": "g", "int_value": {}},
        {"name": "h", "int_value": {}},
        {"name": "i", "double_value": {"required": true}},
        {"name": "j", "double_value": {}},
        {"name": "k", "double_value": {"required": true}}
      ]
    }
  )"_json);

  std::vector<std::string> expected_errors{
      "Arguments at index [1, 2, 4, 5, 6, 8] need to also be required, because "
      "arguments preceding a required argument must be requried to prevent "
      "ambiguous parsing."};

  expect_validate_errors(descriptor, expected_errors);
}
