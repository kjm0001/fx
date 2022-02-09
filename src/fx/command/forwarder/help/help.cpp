#include "help.hpp"
#include <fmt/color.h>
#include <fmt/core.h>
#include <boost/algorithm/string_regex.hpp>
#include <boost/regex.hpp>
#include <sstream>

// This is a complete mess. Like seriously. Gotta ship tho. CLEAN IT UP LATER!!!
namespace fx::command::forwarder::help {
  static const std::string::size_type WIDTH = 80;
  static const std::string::size_type INDENT = 3;

  fx::result::Result<void> print(
      const std::string& command_name,
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    fmt::print("{0} — {1}\n\n",
               fmt::format(fmt::emphasis::bold, "fx {0}", command_name),
               descriptor.synopsis());

    fmt::print("{}\n", wrap("usage:", INDENT));
    fmt::print("{}\n\n", wrap(usage(command_name, descriptor), INDENT * 2));

    if (!descriptor.description().empty()) {
      fmt::print("{0}\n\n", wrap(descriptor.description(), 0));
    }

    if (!descriptor.options().empty()) {
      fmt::print("{0:·<{1}}\n\n", "· OPTIONS ", WIDTH);
      for (const auto& option : descriptor.options()) {
        coerce_value(option, [&](auto value) {
          std::vector<std::string> header_components{};
          std::string name = fmt::format("{0:>{1}}", "--" + option.name(),
                                         option.name().size() + INDENT + 2);
          if (!option.short_name().empty()) {
            name = fmt::format("{0}, -{1}", name, option.short_name());
          }
          header_components.emplace_back(name);
          header_components.emplace_back(
              fmt::format(value.list ? "List<{}>" : "{}", value.type_name));
          header_components.emplace_back(value.required ? "required"
                                                        : "optional");
          fmt::print("{}\n\n", fmt::join(header_components, " · "));

          fmt::print("{}\n", wrap(option.description(), INDENT * 2));

          std::vector<std::string> footer_components{};
          footer_components.emplace_back(
              fmt::format("Default: {}", value.default_value));
          if (!value.choices.empty()) {
            footer_components.emplace_back(
                fmt::format("Choices: {0}", fmt::join(value.choices, ", ")));
          }
          fmt::print(
              "\n{}\n",
              wrap(fmt::format("{}", fmt::join(footer_components, " | ")),
                   INDENT * 2));
        });
        fmt::print("\n");
      }
    }

    if (!descriptor.arguments().empty()) {
      fmt::print("{0:·<{1}}\n\n", "· ARGUMENTS ", WIDTH);
      for (const auto& argument : descriptor.arguments()) {
        coerce_value(argument, [&](auto value) {
          std::vector<std::string> header_components{};
          std::string name = fmt::format("{0:>{1}}", argument.name(),
                                         argument.name().size() + INDENT);
          header_components.emplace_back(name);
          header_components.emplace_back(
              fmt::format(value.list ? "List<{}>" : "{}", value.type_name));
          header_components.emplace_back(value.required ? "required"
                                                        : "optional");
          fmt::print("{}\n\n", fmt::join(header_components, " · "));

          fmt::print("{}\n", wrap(argument.description(), INDENT * 2));

          std::vector<std::string> footer_components{};
          footer_components.emplace_back(
              fmt::format("Default: {}", value.default_value));
          if (!value.choices.empty()) {
            footer_components.emplace_back(
                fmt::format("Choices: {0}", fmt::join(value.choices, ", ")));
          }
          fmt::print(
              "\n{}\n",
              wrap(fmt::format("{}", fmt::join(footer_components, " | ")),
                   INDENT * 2));
        });

        fmt::print("\n");
      }
    }

    return fx::result::Ok();
  }

  std::string usage(
      const std::string& command_name,
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor) {
    std::vector<std::string> components{"fx", command_name};

    for (const auto& option : descriptor.options()) {
      coerce_value(option, [&](auto value) {
        std::vector<std::string> subcomps{};
        if (!option.short_name().empty()) {
          subcomps.emplace_back(fmt::format("-{0}", option.short_name()));
        }
        subcomps.emplace_back(fmt::format(
            "--{0}{1}", option.name(),
            value.takes_value ? fmt::format("=<{0}{1}>", option.name(),
                                            value.list ? "..." : "")
                              : ""));
        components.emplace_back(fmt::format(value.required ? "{}" : "[{}]",
                                            fmt::join(subcomps, "|")));
      });
    }

    for (const auto& argument : descriptor.arguments()) {
      coerce_value(argument, [&](auto value) {
        const auto arg =
            fmt::format("{0}{1}", argument.name(), value.list ? "..." : "");
        components.emplace_back(
            fmt::format(value.required ? "{}" : "[{}]", arg));
      });
    }

    return fmt::format("{}", fmt::join(components, " "));
  }

  std::string wrap(const std::string& content,
                   std::string::size_type left_padding) {
    std::string padding = fmt::format("{0:<{1}}", "", left_padding);
    std::string result{};
    std::string::size_type max_width = WIDTH - left_padding;

    std::vector<std::string> lines;
    boost::algorithm::split_regex(lines, content, boost::regex("\n"));

    for (const auto& line : lines) {
      if (line.empty()) {
        result += "\n";
      } else {
        std::vector<std::string> words;
        boost::algorithm::split(words, line, boost::is_any_of("\t "));
        std::string::size_type current_line_length = 0;
        result += padding;
        for (const auto& word : words) {
          const auto padded_word = fmt::format("{} ", word);
          if (current_line_length + padded_word.size() > max_width) {
            result += fmt::format("\n{}", padding);
            current_line_length = 0;
          }
          result += padded_word;
          current_line_length += padded_word.size();
        }
      }
    }

    return result;
  }

  template <typename F>
  void coerce_value(const fx::descriptor::v1beta::OptionDescriptor& descriptor,
                    F lambda) {
    if (descriptor.has_bool_value()) {
      coerced_value_t<bool> coerced_value;
      coerced_value.type_name = "bool";
      coerced_value.choices = std::vector<bool>{};
      coerced_value.required = false;
      coerced_value.list = false;
      coerced_value.default_value = false;
      coerced_value.takes_value = false;

      lambda(coerced_value);
    } else if (descriptor.has_int_value()) {
      coerced_value_t<int64_t> coerced_value;
      coerced_value.type_name = "int";
      coerced_value.choices =
          std::vector<int64_t>{descriptor.int_value().choices().begin(),
                               descriptor.int_value().choices().end()};
      coerced_value.required = descriptor.int_value().required();
      coerced_value.list = descriptor.int_value().list();
      coerced_value.default_value = descriptor.int_value().default_();
      coerced_value.takes_value = true;

      lambda(coerced_value);
    } else if (descriptor.has_double_value()) {
      coerced_value_t<double> coerced_value;
      coerced_value.type_name = "double";
      coerced_value.choices =
          std::vector<double>{descriptor.double_value().choices().begin(),
                              descriptor.double_value().choices().end()};
      coerced_value.required = descriptor.double_value().required();
      coerced_value.list = descriptor.double_value().list();
      coerced_value.default_value = descriptor.double_value().default_();
      coerced_value.takes_value = true;

      lambda(coerced_value);
    } else if (descriptor.has_string_value()) {
      coerced_value_t<std::string> coerced_value;
      coerced_value.type_name = "string";
      coerced_value.choices =
          std::vector<std::string>{descriptor.string_value().choices().begin(),
                                   descriptor.string_value().choices().end()};
      coerced_value.required = descriptor.string_value().required();
      coerced_value.list = descriptor.string_value().list();
      coerced_value.default_value = descriptor.string_value().default_();
      coerced_value.takes_value = true;

      lambda(coerced_value);
    }
  }

  template <typename F>
  void coerce_value(
      const fx::descriptor::v1beta::ArgumentDescriptor& descriptor, F lambda) {
    if (descriptor.has_int_value()) {
      coerced_value_t<int64_t> coerced_value;
      coerced_value.type_name = "int";
      coerced_value.choices =
          std::vector<int64_t>{descriptor.int_value().choices().begin(),
                               descriptor.int_value().choices().end()};
      coerced_value.required = descriptor.int_value().required();
      coerced_value.list = descriptor.int_value().list();
      coerced_value.default_value = descriptor.int_value().default_();
      coerced_value.takes_value = false;

      lambda(coerced_value);
    } else if (descriptor.has_double_value()) {
      coerced_value_t<double> coerced_value;
      coerced_value.type_name = "double";
      coerced_value.choices =
          std::vector<double>{descriptor.double_value().choices().begin(),
                              descriptor.double_value().choices().end()};
      coerced_value.required = descriptor.double_value().required();
      coerced_value.list = descriptor.double_value().list();
      coerced_value.default_value = descriptor.double_value().default_();
      coerced_value.takes_value = false;

      lambda(coerced_value);
    } else if (descriptor.has_string_value()) {
      coerced_value_t<std::string> coerced_value;
      coerced_value.type_name = "string";
      coerced_value.choices =
          std::vector<std::string>{descriptor.string_value().choices().begin(),
                                   descriptor.string_value().choices().end()};
      coerced_value.required = descriptor.string_value().required();
      coerced_value.list = descriptor.string_value().list();
      coerced_value.default_value = descriptor.string_value().default_();
      coerced_value.takes_value = false;

      lambda(coerced_value);
    }
  }
}  // namespace fx::command::forwarder::help
