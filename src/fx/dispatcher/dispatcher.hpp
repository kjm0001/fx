#pragma once

#include <memory>
#include <string>
#include <vector>
#include "fx/command/base/base.hpp"

namespace fx::dispatcher {
  // Protocol ------------------------------------------------------------------

  class Protocol {
   public:
    Protocol();
    void dispatch();
    const std::vector<std::string>& arguments();
    std::shared_ptr<fx::command::Base> command();

    virtual ~Protocol() = 0;

   protected:
    std::vector<std::string> _arguments;
    std::shared_ptr<fx::command::Base> _command;
  };

  // Dispatcher ----------------------------------------------------------------

  class Dispatcher : public Protocol {
   public:
    Dispatcher(const std::vector<std::string>& arguments);
  };
}  // namespace fx::dispatcher
