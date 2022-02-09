#pragma once

#include <string>
#include "fx/descriptor/v1beta/descriptor.pb.h"
#include "fx/result/result.hpp"

namespace fx::command::forwarder::help {
  template <typename T>
  struct coerced_value_t {
    std::string type_name;
    std::vector<T> choices;
    bool required;
    bool list;
    T default_value;
    bool takes_value;
  };

  fx::result::Result<void> print(
      const std::string& command_name,
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor);

  std::string usage(
      const std::string& command_name,
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor);

  std::string wrap(const std::string& content,
                   std::string::size_type left_padding);

  template <typename F>
  void coerce_value(const fx::descriptor::v1beta::OptionDescriptor& descriptor,
                    F lambda);

  template <typename F>
  void coerce_value(
      const fx::descriptor::v1beta::ArgumentDescriptor& descriptor, F lambda);
}  // namespace fx::command::forwarder::help
