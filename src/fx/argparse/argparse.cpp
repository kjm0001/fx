#include "argparse.hpp"
#include <fmt/core.h>
#include <lyra/lyra.hpp>
#include "fx/argparse/converter/converter.hpp"

namespace fx::argparse {
  fx::result::Result<nlohmann::json> parse(
      const std::vector<std::string> &arguments,
      const fx::descriptor::v1beta::FxCommandDescriptor &descriptor) {
    converter::named_opt_value_t values;
    const auto cli = converter::create_lyra_cli(descriptor, values);

    // Lyra expects the first argument to be the command name. So, insert a
    // fake command when parsing the args.
    std::vector<std::string> enriched_arguments{"fx"};
    enriched_arguments.insert(enriched_arguments.end(), arguments.begin(),
                              arguments.end());
    auto result = cli.parse(
        lyra::args{enriched_arguments.begin(), enriched_arguments.end()});
    if (!result) {
      return fx::result::Error(fmt::format("[argparse] {0}", result.message()));
    }

    return fx::result::Ok(converter::values_to_json(values));
  }
}  // namespace fx::argparse
