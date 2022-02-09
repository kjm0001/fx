#pragma once

#include <nlohmann/json.hpp>
#include "fx/descriptor/v1beta/descriptor.pb.h"
#include "fx/result/result.hpp"

namespace fx::argparse {
  fx::result::Result<nlohmann::json> parse(
      const std::vector<std::string>& arguments,
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor);
}  // namespace fx::argparse
