#pragma once

#include "fx/command/base/base.hpp"
#include "fx/result/result.hpp"

namespace fx::command {
  class Version : public fx::command::Base {
   public:
    Version();
    fx::result::Result<void> run(
        const std::vector<std::string>& arguments) override;
  };
}  // namespace fx::command
