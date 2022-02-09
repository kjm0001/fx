#include "validator.hpp"
#include <fmt/core.h>
#include <fmt/ranges.h>
#include "fx/util/util.hpp"

namespace fx::parser::validator {
  fx::result::Result<void> validate_descriptor(
      const fx::descriptor::v1beta::FxWorkspaceDescriptor& descriptor) {
    std::vector<std::string> error_messages{};

    const std::string supported_version("v1beta");
    if (descriptor.descriptor_version() != supported_version) {
      error_messages.emplace_back(fmt::format(
          "Unsupported descriptor version \"{0}\", only {1} is supported.",
          descriptor.descriptor_version(), supported_version));
    }

    if (!error_messages.empty()) {
      return fx::result::Error(
          fmt::format("{0}", fmt::join(error_messages, " ")));
    }

    return fx::result::Ok();
  }

  fx::result::Result<void> validate_descriptor(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    std::vector<std::string> error_messages{};

    validate_command(descriptor, error_messages);
    validate_options(descriptor.options(), error_messages);
    validate_arguments(descriptor.arguments(), error_messages);
    validate_unique_name_constraint(descriptor.options(),
                                    descriptor.arguments(), error_messages);

    if (!error_messages.empty()) {
      return fx::result::Error(
          fmt::format("{0}", fmt::join(error_messages, " ")));
    }

    return fx::result::Ok();
  }

  void validate_command(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      std::vector<std::string>& error_messages) {
    const std::string supported_version("v1beta");
    if (descriptor.descriptor_version() != supported_version) {
      error_messages.emplace_back(fmt::format(
          "Unsupported descriptor version \"{0}\", only {1} is supported.",
          descriptor.descriptor_version(), supported_version));
    }

    if (descriptor.synopsis().empty()) {
      error_messages.emplace_back("Command synopsis cannot be empty.");
    }

    if (descriptor.runtime().run().empty()) {
      error_messages.emplace_back("Runtime run cannot be empty.");
    }
  }

  void validate_options(const google::protobuf::RepeatedPtrField<
                            fx::descriptor::v1beta::OptionDescriptor>& options,
                        std::vector<std::string>& error_messages) {
    for (int index = 0; index < options.size(); index++) {
      const auto& option = options[index];
      const auto error_prefix = fmt::format("Option[index:{0}]", index);

      validate_name(option, error_prefix, error_messages);
      validate_short_name(option, error_prefix, error_messages);
      validate_description(option, error_prefix, error_messages);
      validate_value(option, error_prefix, error_messages);
    }
  }

  void validate_arguments(
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::ArgumentDescriptor>& arguments,
      std::vector<std::string>& error_messages) {
    for (int index = 0; index < arguments.size(); index++) {
      const auto& argument = arguments[index];
      const auto error_prefix = fmt::format("Argument[index:{0}]", index);

      validate_name(argument, error_prefix, error_messages);
      validate_description(argument, error_prefix, error_messages);
      validate_value(argument, error_prefix, error_messages);
    }

    validate_argument_list_constraint(arguments, error_messages);
    validate_argument_require_constraint(arguments, error_messages);
  }

  template <typename D>
  void validate_name(const D& descriptor, const std::string& error_prefix,
                     std::vector<std::string>& error_messages) {
    if (descriptor.name().size() <= 1) {
      error_messages.emplace_back(
          fmt::format("{0} \"{1}\" name cannot be shorter than two characters.",
                      error_prefix, descriptor.name()));
    }

    if (fx::util::icompare(descriptor.name(), "help")) {
      error_messages.emplace_back(fmt::format(
          R"({0} name cannot be named "help" (reserved).)", error_prefix));
    }
  }

  void validate_short_name(
      const fx::descriptor::v1beta::OptionDescriptor& descriptor,
      const std::string& error_prefix,
      std::vector<std::string>& error_messages) {
    if (descriptor.short_name().size() > 1) {
      error_messages.emplace_back(fmt::format(
          "{0} \"{1}\" short name cannot be longer than one character.",
          error_prefix, descriptor.short_name()));
    }

    if (fx::util::icompare(descriptor.short_name(), "h")) {
      error_messages.emplace_back(
          fmt::format(R"({0} "{1}" short name cannot be "h" (reserved).)",
                      error_prefix, descriptor.name()));
    }
  }

  template <typename D>
  void validate_description(const D& descriptor,
                            const std::string& error_prefix,
                            std::vector<std::string>& error_messages) {
    if (descriptor.description().empty()) {
      error_messages.emplace_back(
          fmt::format("{0} \"{1}\" description cannot be empty.", error_prefix,
                      descriptor.name()));
    }
  }

  template <typename D>
  void validate_value(const D& descriptor, const std::string& error_prefix,
                      std::vector<std::string>& error_messages) {
    if (descriptor.value_case() == D::ValueCase::VALUE_NOT_SET) {
      error_messages.emplace_back(
          fmt::format("{0} \"{1}\" value cannot be empty.", error_prefix,
                      descriptor.name()));
    } else {
      coerce_value(descriptor, [&](auto value) {
        if (!value.choices.empty() &&
            (std::find(value.choices.begin(), value.choices.end(),
                       value.default_value) == value.choices.end())) {
          error_messages.emplace_back(fmt::format(
              "{0} \"{1}\" default value \"{2}\" is invalid, the default "
              "value must be one of the choices: [{3}].",
              error_prefix, descriptor.name(), value.default_value,
              fmt::join(value.choices, ", ")));
        }
      });
    }
  }

  void validate_argument_list_constraint(
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::ArgumentDescriptor>& arguments,
      std::vector<std::string>& error_messages) {
    std::vector<int> list_indicies{};
    for (int index = 0; index < arguments.size(); index++) {
      const auto& argument = arguments[index];
      coerce_value(argument, [&](auto value) {
        if (value.list) {
          list_indicies.push_back(index);
        }
      });
    }

    if (list_indicies.size() > 1 ||
        (list_indicies.size() == 1 &&
         list_indicies[0] != arguments.size() - 1)) {
      error_messages.emplace_back(
          "To prevent ambiguous argument parsing, only the last argument can "
          "be a list.");
    }
  }

  void validate_argument_require_constraint(
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::ArgumentDescriptor>& arguments,
      std::vector<std::string>& error_messages) {
    std::set<int> required_to_be_required{};

    int last_required_seen = -1;
    for (int current = 0; current < arguments.size(); current++) {
      coerce_value(arguments[current], [&](auto value) {
        if (value.required) {
          for (int check = last_required_seen + 1; check < current; check++) {
            required_to_be_required.insert(check);
          }

          last_required_seen = current;
        }
      });
    }

    if (!required_to_be_required.empty()) {
      error_messages.emplace_back(
          fmt::format("Arguments at index [{0}] need to also be required, "
                      "because arguments preceding a required argument must be "
                      "requried to prevent ambiguous parsing.",
                      fmt::join(required_to_be_required, ", ")));
    }
  }

  void validate_unique_name_constraint(
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::OptionDescriptor>& options,
      const google::protobuf::RepeatedPtrField<
          fx::descriptor::v1beta::ArgumentDescriptor>& arguments,
      std::vector<std::string>& error_messages) {
    std::unordered_map<std::string, std::vector<std::string>> seen_names{};
    std::unordered_map<std::string, std::vector<std::string>>
        seen_short_names{};

    for (int index = 0; index < options.size(); index++) {
      const auto& option = options[index];

      const auto name = fx::util::lower(option.name());
      seen_names[name].emplace_back(fmt::format("Option[index:{0}]", index));

      const auto short_name = fx::util::lower(option.short_name());
      if (!short_name.empty()) {
        seen_short_names[short_name].emplace_back(
            fmt::format("Option[index:{0}]", index));
      }
    }

    for (int index = 0; index < arguments.size(); index++) {
      const auto& argument = arguments[index];

      const auto name = fx::util::lower(argument.name());
      seen_names[name].emplace_back(fmt::format("Argument[index:{0}]", index));
    }

    for (const auto& [name, indicies] : seen_names) {
      if (indicies.size() > 1) {
        error_messages.emplace_back(
            fmt::format("Name \"{0}\" used in {1} is not unique.", name,
                        fmt::join(indicies, ", ")));
      }
    }

    for (const auto& [short_name, indicies] : seen_short_names) {
      if (indicies.size() > 1) {
        error_messages.emplace_back(
            fmt::format("Short name \"{0}\" used in {1} is not unique.",
                        short_name, fmt::join(indicies, ", ")));
      }
    }
  }

  template <typename D, typename F>
  void coerce_value(const D& descriptor, F lambda) {
    if (descriptor.has_int_value()) {
      coerced_value_t<int64_t> coerced_value;
      coerced_value.choices =
          std::vector<int64_t>{descriptor.int_value().choices().begin(),
                               descriptor.int_value().choices().end()};
      coerced_value.required = descriptor.int_value().required();
      coerced_value.list = descriptor.int_value().list();
      coerced_value.default_value = descriptor.int_value().default_();

      lambda(coerced_value);
    } else if (descriptor.has_double_value()) {
      coerced_value_t<double> coerced_value;
      coerced_value.choices =
          std::vector<double>{descriptor.double_value().choices().begin(),
                              descriptor.double_value().choices().end()};
      coerced_value.required = descriptor.double_value().required();
      coerced_value.list = descriptor.double_value().list();
      coerced_value.default_value = descriptor.double_value().default_();

      lambda(coerced_value);
    } else if (descriptor.has_string_value()) {
      coerced_value_t<std::string> coerced_value;
      coerced_value.choices =
          std::vector<std::string>{descriptor.string_value().choices().begin(),
                                   descriptor.string_value().choices().end()};
      coerced_value.required = descriptor.string_value().required();
      coerced_value.list = descriptor.string_value().list();
      coerced_value.default_value = descriptor.string_value().default_();

      lambda(coerced_value);
    }
  }
}  // namespace fx::parser::validator
