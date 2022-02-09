#pragma once

#include <lyra/lyra.hpp>
#include <nlohmann/json.hpp>
#include <set>
#include <variant>
#include "fx/descriptor/v1beta/descriptor.pb.h"

namespace fx::argparse::converter {
  struct opt_value_t {
    bool user_set;
    std::variant<bool, std::string, std::vector<std::string>, int64_t,
                 std::vector<int64_t>, double, std::vector<double>>
        value;
  };

  typedef std::unordered_map<std::string, opt_value_t> named_opt_value_t;

  template <typename T>
  struct opt_input_t {
    std::set<T> choices;
    bool required;
    std::variant<T, std::vector<T>> default_value;
  };

  template <typename D, typename I>
  I enrich_input(const D& descriptor, I& input) {
    return input;
  }

  template <typename I, typename T, typename D>
  I create_input(const D& descriptor, named_opt_value_t& values,
                 T default_value, bool required, const std::set<T>& choices);

  template <typename I, typename T, typename D>
  I create_input(const D& descriptor, named_opt_value_t& values,
                 std::vector<T> default_values, bool required,
                 std::set<T> choices);

  lyra::opt create_bool_option(
      const fx::descriptor::v1beta::OptionDescriptor& descriptor,
      named_opt_value_t& values);

  template <typename D>
  std::variant<opt_input_t<int64_t>, opt_input_t<double>,
               opt_input_t<std::string>>
  create_opt_input_t(const D& descriptor);

  lyra::cli create_lyra_cli(
      const fx::descriptor::v1beta::FxCommandDescriptor& descriptor,
      named_opt_value_t& values);

  nlohmann::json values_to_json(const named_opt_value_t& values);
}  // namespace fx::argparse::converter
