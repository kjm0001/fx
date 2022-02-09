#include "converter.hpp"

namespace fx::argparse::converter {
  template <>
  lyra::opt enrich_input(
      const fx::descriptor::v1beta::OptionDescriptor &descriptor,
      lyra::opt &input) {
    if (!descriptor.short_name().empty()) {
      input = input.name("-" + descriptor.short_name());
    }
    return input.name("--" + descriptor.name()).help(descriptor.description());
  }

  template <typename I, typename T, typename D>
  I create_input(const D &descriptor, named_opt_value_t &values,
                 T default_value, bool required, const std::set<T> &choices) {
    static_assert(std::is_base_of<lyra::bound_parser<I>, I>::value,
                  "type I must be lyra::arg or lyra::opt");

    values[descriptor.name()] = opt_value_t{false, default_value};
    auto input = I(
                     [&](T new_value) {
                       opt_value_t &opt_value = values[descriptor.name()];
                       opt_value.user_set = true;
                       opt_value.value = new_value;
                     },
                     descriptor.name())
                     .cardinality(required ? 1 : 0, 1);

    if (!choices.empty()) {
      input = input.choices([=](T choice) {
        return choices.find(choice) != choices.end();
      });
    }

    return enrich_input(descriptor, input);
  }

  template <typename I, typename T, typename D>
  I create_input(const D &descriptor, named_opt_value_t &values,
                 std::vector<T> default_values, bool required,
                 std::set<T> choices) {
    static_assert(std::is_base_of<lyra::bound_parser<I>, I>::value,
                  "type I must be lyra::arg or lyra::opt");
    values[descriptor.name()] = opt_value_t{false, default_values};
    auto input = I(
                     [&](T new_value) {
                       opt_value_t &opt_value = values[descriptor.name()];
                       auto &current =
                           std::get<std::vector<T>>(opt_value.value);
                       if (!opt_value.user_set) {
                         // The user is setting the input, so clear the
                         // default values.
                         current.clear();
                       }
                       opt_value.user_set = true;
                       current.push_back(new_value);
                     },
                     descriptor.name())
                     .cardinality(required ? 1 : 0, 0);

    if (choices.size() > 0) {
      input = input.choices([=](T choice) {
        return choices.find(choice) != choices.end();
      });
    }

    return enrich_input(descriptor, input);
  }

  lyra::opt create_bool_option(
      const fx::descriptor::v1beta::OptionDescriptor &descriptor,
      named_opt_value_t &values) {
    values[descriptor.name()] = opt_value_t{false, false};
    auto option = lyra::opt([&](bool new_value) {
      opt_value_t &opt_value = values[descriptor.name()];
      opt_value.user_set = true;
      opt_value.value = new_value;
    });
    return enrich_input(descriptor, option);
  }

  template <typename D>
  std::variant<opt_input_t<int64_t>, opt_input_t<double>,
               opt_input_t<std::string>>
  create_opt_input_t(const D &descriptor) {
    if (descriptor.has_int_value()) {
      opt_input_t<int64_t> oit;
      oit.choices = std::set<int64_t>{descriptor.int_value().choices().begin(),
                                      descriptor.int_value().choices().end()};
      oit.required = descriptor.int_value().required();
      if (descriptor.int_value().list()) {
        oit.default_value = std::vector{descriptor.int_value().default_()};
      } else {
        oit.default_value = descriptor.int_value().default_();
      }
      return oit;
    } else if (descriptor.has_double_value()) {
      opt_input_t<double> oit;
      oit.choices =
          std::set<double>{descriptor.double_value().choices().begin(),
                           descriptor.double_value().choices().end()};
      oit.required = descriptor.double_value().required();
      if (descriptor.double_value().list()) {
        oit.default_value = std::vector{descriptor.double_value().default_()};
      } else {
        oit.default_value = descriptor.double_value().default_();
      }
      return oit;
    } else if (descriptor.has_string_value()) {
      opt_input_t<std::string> oit;
      oit.choices =
          std::set<std::string>{descriptor.string_value().choices().begin(),
                                descriptor.string_value().choices().end()};
      oit.required = descriptor.string_value().required();
      if (descriptor.string_value().list()) {
        oit.default_value = std::vector{descriptor.string_value().default_()};
      } else {
        oit.default_value = descriptor.string_value().default_();
      }
      return oit;
    }

    // We should never reach this case. This is here to satisfy the compiler.
    exit(1);
  }

  lyra::cli create_lyra_cli(
      const fx::descriptor::v1beta::FxCommandDescriptor &descriptor,
      named_opt_value_t &values) {
    auto cli = lyra::cli();

    values["help"] = opt_value_t{false, false};
    cli.add_argument(lyra::help(std::get<bool>(values["help"].value)));

    for (const auto &option_descriptor : descriptor.options()) {
      if (option_descriptor.has_bool_value()) {
        cli.add_argument(create_bool_option(option_descriptor, values));
      } else {
        std::visit(
            [&](auto oit) {
              std::visit(
                  [&](auto default_value) {
                    cli.add_argument(create_input<lyra::opt>(
                        option_descriptor, values, default_value, oit.required,
                        oit.choices));
                  },
                  oit.default_value);
            },
            create_opt_input_t(option_descriptor));
      }
    }

    for (const auto &argument_descriptor : descriptor.arguments()) {
      std::visit(
          [&](auto oit) {
            std::visit(
                [&](auto default_value) {
                  cli.add_argument(create_input<lyra::arg>(
                      argument_descriptor, values, default_value, oit.required,
                      oit.choices));
                },
                oit.default_value);
          },
          create_opt_input_t(argument_descriptor));
    }

    return cli;
  }

  nlohmann::json values_to_json(const named_opt_value_t &values) {
    auto json = nlohmann::json::object();

    for (const auto &[option_name, option_value] : values) {
      auto &option_json = json[option_name];
      option_json["user_set"] = option_value.user_set;
      std::visit(
          [&](auto value) {
            option_json["value"] = value;
          },
          option_value.value);
    }

    return json;
  }
}  // namespace fx::argparse::converter
