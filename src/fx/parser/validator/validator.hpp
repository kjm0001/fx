#pragma once

#include <unordered_map>
#include <variant>
#include "fx/descriptor/v1beta/descriptor.pb.h"
#include "fx/result/result.hpp"

namespace fx::parser::validator {
  template <typename T>
  struct coerced_value_t {
    std::vector<T> choices;
    bool required;
    bool list;
    T default_value;
  };

  fx::result::Result<void> validate_descriptor(
      const fx::descriptor::v1beta::FxWorkspaceDescriptor& descriptor);

  fx::result::Result<void> validate_descriptor(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor);

  void validate_command(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      std::vector<std::string>& error_messages);

  void validate_options(const google::protobuf::RepeatedPtrField<
                            fx::descriptor::v1beta::OptionDescriptor>& options,
                        std::vector<std::string>& error_messages);

  void validate_arguments(
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::ArgumentDescriptor>& arguments,
      std::vector<std::string>& error_messages);

  template <typename D>
  void validate_name(const D& descriptor, const std::string& error_prefix,
                     std::vector<std::string>& error_messages);

  void validate_short_name(
      const fx::descriptor::v1beta::OptionDescriptor& descriptor,
      const std::string& error_prefix,
      std::vector<std::string>& error_messages);

  template <typename D>
  void validate_description(const D& descriptor,
                            const std::string& error_prefix,
                            std::vector<std::string>& error_messages);

  template <typename D>
  void validate_value(const D& descriptor, const std::string& error_prefix,
                      std::vector<std::string>& error_messages);

  void validate_argument_list_constraint(
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::ArgumentDescriptor>& arguments,
      std::vector<std::string>& error_messages);

  void validate_argument_require_constraint(
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::ArgumentDescriptor>& arguments,
      std::vector<std::string>& error_messages);

  void validate_unique_name_constraint(
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::OptionDescriptor>& options,
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::ArgumentDescriptor>& arguments,
      std::vector<std::string>& error_messages);

  template <typename D, typename F>
  void coerce_value(const D& descriptor, F lambda);
}  // namespace fx::parser::validator
