#pragma once

#include <string>
#include <vector>
#include "fx/result/result.hpp"

namespace fx::command {
  class Base {
   public:
    Base();
    virtual ~Base() = 0;
    virtual fx::result::Result<void> run(
        const std::vector<std::string>& arguments) = 0;
  };
}  // namespace fx::command
