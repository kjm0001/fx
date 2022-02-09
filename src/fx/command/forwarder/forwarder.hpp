#pragma once

#include <filesystem>
#include <nlohmann/json.hpp>
#include "fx/command/base/base.hpp"
#include "fx/descriptor/v1beta/descriptor.pb.h"
#include "fx/result/result.hpp"

namespace fx::command {
  class Protocol {
   public:
    Protocol() = default;

    virtual ~Protocol() = 0;

    virtual fx::result::Result<std::filesystem::path>
    find_workspace_descriptor_path() = 0;

    virtual fx::result::Result<fx::descriptor::v1beta::FxCommandDescriptor>
    parse_command_descriptor(
        const std::filesystem::path& workspace_descriptor_path) = 0;

    virtual fx::result::Result<nlohmann::json> parse_command_arguments(
        const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
        const std::vector<std::string>& arguments) = 0;

    virtual std::string shell() = 0;

    virtual std::vector<std::string> collate_execution_arguments(
        const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
        const nlohmann::json& arguments) = 0;

    virtual std::vector<std::string> collate_enviornment_variables(
        const std::filesystem::path& workspace_descriptor_path) = 0;

    virtual fx::result::Result<void> execute_command(
        const std::vector<std::string>& arguments,
        const std::vector<std::string>& envvars) = 0;

    virtual fx::result::Result<void> execute_help(
        const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) = 0;
  };

  class Forwarder : public Protocol, public fx::command::Base {
   public:
    Forwarder(const std::string& command_name);

    fx::result::Result<void> run(
        const std::vector<std::string>& arguments) override;

    fx::result::Result<std::filesystem::path> find_workspace_descriptor_path()
        override;

    fx::result::Result<fx::descriptor::v1beta::FxCommandDescriptor>
    parse_command_descriptor(
        const std::filesystem::path& workspace_descriptor_path) override;

    fx::result::Result<nlohmann::json> parse_command_arguments(
        const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
        const std::vector<std::string>& arguments) override;

    std::string shell() override;

    std::vector<std::string> collate_execution_arguments(
        const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
        const nlohmann::json& arguments) override;

    std::vector<std::string> collate_enviornment_variables(
        const std::filesystem::path& workspace_descriptor_path) override;

    fx::result::Result<void> execute_command(
        const std::vector<std::string>& arguments,
        const std::vector<std::string>& envvars) override;

    fx::result::Result<void> execute_help(
        const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) override;

   private:
    std::string _command_name;
  };
}  // namespace fx::command
