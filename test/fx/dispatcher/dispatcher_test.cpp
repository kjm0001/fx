#include "fx/dispatcher/dispatcher.hpp"
#include <gtest/gtest.h>
#include <memory>
#include "fx/command/base/base.hpp"
#include "fx/command/forwarder/forwarder.hpp"
#include "fx/command/help/help.hpp"
#include "fx/command/list/list.hpp"
#include "fx/command/version/version.hpp"
#include "fx/result/result.hpp"

// Dispatch --------------------------------------------------------------------

struct Dispatch : testing::Test {
  template <typename C>
  void static expect_initialize_eq(
      const std::vector<std::string>& input_arguments,
      const std::vector<std::string>& expected_arguments) {
    const auto dispatcher =
        std::make_shared<fx::dispatcher::Dispatcher>(input_arguments);
    EXPECT_EQ(expected_arguments, dispatcher->arguments());
    EXPECT_NE(std::dynamic_pointer_cast<C>(dispatcher->command()), nullptr);
  }
};

TEST_F(Dispatch, EmptyArguments) {
  const std::vector<std::string> input_arguments{};
  const std::vector<std::string> expected_arguments{};
  expect_initialize_eq<fx::command::List>(input_arguments, expected_arguments);
}

TEST_F(Dispatch, StandardList) {
  const std::vector<std::string> input_arguments{"list"};
  const std::vector<std::string> expected_arguments{};
  expect_initialize_eq<fx::command::List>(input_arguments, expected_arguments);
}

TEST_F(Dispatch, StandardHelp) {
  const std::vector<std::string> input_arguments{"help"};
  const std::vector<std::string> expected_arguments{};
  expect_initialize_eq<fx::command::Help>(input_arguments, expected_arguments);
}

TEST_F(Dispatch, StandardHelpFlag) {
  const std::vector<std::string> input_arguments{"--help"};
  const std::vector<std::string> expected_arguments{};
  expect_initialize_eq<fx::command::Help>(input_arguments, expected_arguments);
}

TEST_F(Dispatch, StandardHFlag) {
  const std::vector<std::string> input_arguments{"-h"};
  const std::vector<std::string> expected_arguments{};
  expect_initialize_eq<fx::command::Help>(input_arguments, expected_arguments);
}

TEST_F(Dispatch, StandardVersion) {
  const std::vector<std::string> input_arguments{"version"};
  const std::vector<std::string> expected_arguments{};
  expect_initialize_eq<fx::command::Version>(input_arguments,
                                             expected_arguments);
}

TEST_F(Dispatch, ForwarderDispatch) {
  const std::vector<std::string> input_arguments{"tools/example", "--option",
                                                 "abc", "arg1", "arg2"};
  const std::vector<std::string> expected_arguments{"--option", "abc", "arg1",
                                                    "arg2"};
  expect_initialize_eq<fx::command::Forwarder>(input_arguments,
                                               expected_arguments);
}

// DispatchCommand -------------------------------------------------------------

class TestDispatcher : public fx::dispatcher::Protocol {
 public:
  TestDispatcher(const std::shared_ptr<fx::command::Base>& command,
                 const std::vector<std::string>& arguments) {
    _command = command;
    _arguments = arguments;
  }
};

class SuccessfulCommand : public fx::command::Base {
 public:
  fx::result::Result<void> run(
      const std::vector<std::string>& arguments) override {
    std::vector<std::string> expected{"test", "ok"};
    EXPECT_EQ(expected, arguments);
    return fx::result::Ok();
  }
};

TEST(DispatchCommand, SuccessfulRun) {
  const std::vector<std::string> arguments{"test", "ok"};
  const auto command = std::make_shared<SuccessfulCommand>();
  const auto dispatcher = std::make_shared<TestDispatcher>(command, arguments);
  dispatcher->dispatch();
}

class FailingCommand : public fx::command::Base {
 public:
  fx::result::Result<void> run(
      const std::vector<std::string>& /*arguments*/) override {
    return fx::result::Error(std::string("intentional test error"));
  }
};

TEST(DispatchCommand, FailingRun) {
  const std::vector<std::string> arguments{};
  const auto command = std::make_shared<FailingCommand>();
  const auto dispatcher = std::make_shared<TestDispatcher>(command, arguments);
  ASSERT_DEATH({ dispatcher->dispatch(); }, "");
}
