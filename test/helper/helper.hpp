#pragma once

#include <nlohmann/json.hpp>
#include "fx/descriptor/v1beta/descriptor.pb.h"

namespace fx::test::helper {

  template <typename D>
  D proto_from_json(const nlohmann::json &json);

  fx::descriptor::v1beta::FxWorkspaceDescriptor workspace_descriptor(
      const nlohmann::json &json);

  fx::descriptor::v1beta::FxCommandDescriptor command_descriptor(
      const nlohmann::json &json);

  fx::descriptor::v1beta::OptionDescriptor option_descriptor(
      const nlohmann::json &json);

  fx::descriptor::v1beta::ArgumentDescriptor argument_descriptor(
      const nlohmann::json &json);
}  // namespace fx::test::helper
