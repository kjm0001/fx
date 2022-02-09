#include "helper.hpp"
#include <google/protobuf/util/json_util.h>
#include <gtest/gtest.h>

namespace fx::test::helper {

  template <typename D>
  D proto_from_json(const nlohmann::json &json) {
    D descriptor;
    const auto status = google::protobuf::util::JsonStringToMessage(
        json.dump(), &descriptor, {});
    EXPECT_TRUE(status.ok()) << status.ToString();
    return descriptor;
  }

  fx::descriptor::v1beta::FxWorkspaceDescriptor workspace_descriptor(
      const nlohmann::json &json) {
    return proto_from_json<fx::descriptor::v1beta::FxWorkspaceDescriptor>(json);
  }

  fx::descriptor::v1beta::FxCommandDescriptor command_descriptor(
      const nlohmann::json &json) {
    return proto_from_json<fx::descriptor::v1beta::FxCommandDescriptor>(json);
  }

  fx::descriptor::v1beta::OptionDescriptor option_descriptor(
      const nlohmann::json &json) {
    return proto_from_json<fx::descriptor::v1beta::OptionDescriptor>(json);
  }

  fx::descriptor::v1beta::ArgumentDescriptor argument_descriptor(
      const nlohmann::json &json) {
    return proto_from_json<fx::descriptor::v1beta::ArgumentDescriptor>(json);
  }
}  // namespace fx::test::helper
