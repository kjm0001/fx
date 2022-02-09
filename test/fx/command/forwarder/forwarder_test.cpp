#include "fx/command/forwarder/forwarder.hpp"
#include <fmt/core.h>
#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>
#include <cstdlib>
#include <filesystem>
#include <memory>
#include <nlohmann/json.hpp>
#include "fx/result/result.hpp"
#include "test/helper/helper.hpp"

class TestForwarder : public fx::command::Forwarder {
 public:
  explicit TestForwarder(const std::string& command_name)
      : fx::command::Forwarder(command_name) {}

  MOCK_METHOD(fx::result::Result<std::filesystem::path>,
              find_workspace_descriptor_path, (), (override));
  MOCK_METHOD(std::string, shell, (), (override));
  MOCK_METHOD(fx::result::Result<void>, execute_help,
              (const fx::descriptor::v1beta::FxCommandDescriptor& descriptor),
              (override));
  MOCK_METHOD(fx::result::Result<void>, execute_command,
              (const std::vector<std::string>& arguments,
               const std::vector<std::string>& envvars),
              (override));
};

// Run -------------------------------------------------------------------------

TEST(Run, WorkspaceNotFound) {
  const auto forwarder = std::make_unique<TestForwarder>("test");
  EXPECT_CALL(*forwarder, find_workspace_descriptor_path())
      .Times(1)
      .WillRepeatedly(testing::Return(
          fx::result::Error(std::string("Some workspace error."))));

  const auto actual = forwarder->run({});
  ASSERT_TRUE(actual.failed());
  EXPECT_EQ("Some workspace error.", actual.error());
}

TEST(Run, DescriptorNotFound) {
  const auto forwarder = std::make_shared<TestForwarder>("test");
  EXPECT_CALL(*forwarder, find_workspace_descriptor_path())
      .Times(1)
      .WillRepeatedly(testing::Return(fx::result::Ok(
          std::filesystem::current_path() /
          std::filesystem::path(
              "test/fx/command/forwarder/__data__/workpace.fx.yaml"))));

  const auto actual = forwarder->run({});
  ASSERT_TRUE(actual.failed());
  EXPECT_EQ("Unknown command \"test\".", actual.error());
}

TEST(Run, InvalidArguments) {
  const auto forwarder =
      std::make_unique<TestForwarder>("example/FoundValidCommandDescriptor");
  EXPECT_CALL(*forwarder, find_workspace_descriptor_path())
      .Times(1)
      .WillRepeatedly(testing::Return(fx::result::Ok(
          std::filesystem::current_path() /
          std::filesystem::path(
              "test/fx/command/forwarder/__data__/workpace.fx.yaml"))));

  const auto actual = forwarder->run({"--fail-flag"});
  ASSERT_TRUE(actual.failed());
  EXPECT_EQ("[argparse] Unrecognized token: --fail-flag", actual.error());
}

TEST(Run, HelpFlag) {
  const auto forwarder =
      std::make_unique<TestForwarder>("example/FoundValidCommandDescriptor");
  EXPECT_CALL(*forwarder, find_workspace_descriptor_path())
      .Times(1)
      .WillRepeatedly(testing::Return(fx::result::Ok(
          std::filesystem::current_path() /
          std::filesystem::path(
              "test/fx/command/forwarder/__data__/workpace.fx.yaml"))));
  EXPECT_CALL(*forwarder, execute_help(testing::_))
      .Times(1)
      .WillRepeatedly(testing::Return(fx::result::Ok()));

  const auto actual = forwarder->run({"--help"});
  ASSERT_TRUE(actual.ok());
}

TEST(Run, ExecuteCommand) {
  const auto forwarder =
      std::make_unique<TestForwarder>("example/FoundValidCommandDescriptor");
  EXPECT_CALL(*forwarder, find_workspace_descriptor_path())
      .Times(1)
      .WillRepeatedly(testing::Return(fx::result::Ok(
          std::filesystem::current_path() /
          std::filesystem::path(
              "test/fx/command/forwarder/__data__/workpace.fx.yaml"))));
  EXPECT_CALL(*forwarder, execute_command(testing::_, testing::_))
      .Times(1)
      .WillRepeatedly(testing::Return(fx::result::Ok()));

  const auto actual = forwarder->run({});
  ASSERT_TRUE(actual.ok());
}

// ParseCommandDescriptor ------------------------------------------------------

TEST(ParseCommandDescriptor, FoundValidCommandDescriptor) {
  auto workspace_path =
      std::filesystem::current_path() /
      std::filesystem::path(
          "test/fx/command/forwarder/__data__/workpace.fx.yaml");
  const auto expected = fx::test::helper::command_descriptor(R"({
    "descriptor_version": "v1beta",
    "synopsis": "test",
    "runtime": {"run": "test-run"}
  })"_json);

  const auto forwarder = std::make_unique<fx::command::Forwarder>(
      "example/FoundValidCommandDescriptor");
  const auto actual = forwarder->parse_command_descriptor(workspace_path);

  ASSERT_TRUE(actual.ok()) << actual.error();
  google::protobuf::util::MessageDifferencer::Equals(expected, actual.value());
}

TEST(ParseCommandDescriptor, FoundInvalidCommandDescriptor) {
  auto workspace_path =
      std::filesystem::current_path() /
      std::filesystem::path(
          "test/fx/command/forwarder/__data__/workpace.fx.yaml");

  const auto forwarder = std::make_unique<fx::command::Forwarder>(
      "example/FoundInvalidCommandDescriptor");
  const auto actual = forwarder->parse_command_descriptor(workspace_path);

  ASSERT_TRUE(actual.failed());
  EXPECT_EQ(
      fmt::format(
          "Invalid descriptor: {0}. The root YAML type is: scalar. The "
          "root YAML object must be a map or an array.",
          std::string(workspace_path.parent_path() /
                      std::filesystem::path(
                          "example/"
                          "FoundInvalidCommandDescriptor/command.fx.yaml"))),
      actual.error());
}

TEST(ParseCommandDescriptor, CommandDescriptorNotFound) {
  auto workspace_path =
      std::filesystem::current_path() /
      std::filesystem::path(
          "test/fx/command/forwarder/__data__/workpace.fx.yaml");

  const auto forwarder = std::make_unique<fx::command::Forwarder>(
      "example/CommandDescriptorNotFound");
  const auto actual = forwarder->parse_command_descriptor(workspace_path);

  ASSERT_TRUE(actual.failed());
  EXPECT_EQ("Unknown command \"example/CommandDescriptorNotFound\".",
            actual.error());
}

// ParseCommandArguments -------------------------------------------------------

TEST(ParseCommandArguments, ReturnsJson) {
  const auto descriptor = fx::test::helper::command_descriptor(R"({
    "options": [
      {"name": "bool-test", "description": "test", "bool_value": {}}
    ]
  })"_json);
  const std::vector<std::string> arguments{"--bool-test"};
  const auto expected = R"({
    "bool-test": {"user_set": true, "value": true},
    "help": {"user_set": false,"value": false}
  })"_json;

  const auto forwarder = std::make_unique<fx::command::Forwarder>("test");
  const auto actual = forwarder->parse_command_arguments(descriptor, arguments);
  ASSERT_TRUE(actual.ok());
  EXPECT_EQ(expected, actual.value());
}

TEST(ParseCommandArguments, ReturnsError) {
  const auto descriptor = fx::test::helper::command_descriptor(R"({
    "options": [
      {"name": "bool-test", "description": "test", "bool_value": {}}
    ]
  })"_json);
  const std::vector<std::string> arguments{"--fake-flag"};

  const auto forwarder = std::make_unique<fx::command::Forwarder>("test");
  const auto actual = forwarder->parse_command_arguments(descriptor, arguments);

  ASSERT_TRUE(actual.failed());
  EXPECT_EQ("[argparse] Unrecognized token: --fake-flag", actual.error());
}

// CollateExecutionArguments ---------------------------------------------------

TEST(CollateExecutionArguments, ReturnsExecutionCommands) {
  const auto descriptor = fx::test::helper::command_descriptor(R"({
    "runtime": {
      "run": "python3 $FX_WORKSPACE_DIRECTORY/example/example.py"
    }
  })"_json);
  const auto arguments = R"({
    "bool-test": {"user_set": true, "value": true},
    "string-test": {"user_set": true, "value": "hello"},
    "string-list-test": {"user_set": true, "value": ["hello", "world"]},
    "int-test": {"user_set": true, "value": 416},
    "int-list-test": {"user_set": true, "value": [90, 5]},
    "double-test": {"user_set": true, "value": -90.5},
    "double-list-test": {"user_set": true, "value": [-0.4, 1.6]},
    "help": {"user_set": false,"value": false}
  })"_json;
  const std::vector<std::string> expected{
      "/bin/tuna", "-l", "-c",
      R"(python3 $FX_WORKSPACE_DIRECTORY/example/example.py '{"bool-test":{"user_set":true,"value":true},"double-list-test":{"user_set":true,"value":[-0.4,1.6]},"double-test":{"user_set":true,"value":-90.5},"help":{"user_set":false,"value":false},"int-list-test":{"user_set":true,"value":[90,5]},"int-test":{"user_set":true,"value":416},"string-list-test":{"user_set":true,"value":["hello","world"]},"string-test":{"user_set":true,"value":"hello"}}')"};

  const auto forwarder = std::make_unique<TestForwarder>("test");

  EXPECT_CALL(*forwarder, shell())
      .Times(1)
      .WillRepeatedly(testing::Return("/bin/tuna"));

  const auto actual =
      forwarder->collate_execution_arguments(descriptor, arguments);

  EXPECT_EQ(expected, actual);
}

// CollateEnviornmentVariables -------------------------------------------------

TEST(CollateEnviornmentVariables, ContainsWorkspaceAndExistingEnvvar) {
  const auto test_workspace_path =
      std::filesystem::path("test/path/workspace.yaml");
  setenv("FX_TEST_EXAMPLE_ONE", "416", 1);
  setenv("FX_TEST_EXAMPLE_TWO", "YYZ", 1);
  const std::vector<std::string> expected{"FX_WORKSPACE_DIRECTORY=test/path",
                                          "FX_TEST_EXAMPLE_ONE=416",
                                          "FX_TEST_EXAMPLE_TWO=YYZ"};

  const auto forwarder = std::make_unique<fx::command::Forwarder>("test");
  const auto actual =
      forwarder->collate_enviornment_variables(test_workspace_path);

  EXPECT_THAT(actual, testing::IsSupersetOf(expected));
}
