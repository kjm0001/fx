cc_library(
    name = "forwarder",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    strip_include_prefix = "/src",
    visibility = ["//:__subpackages__"],
    deps = [
        "//src/fx/argparse",
        "//src/fx/command/base",
        "//src/fx/command/forwarder/help",
        "//src/fx/parser",
        "//src/fx/result",
        "//src/fx/util",
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "@com_github_fmtlib_fmt//:fmt",
        "@com_github_gabime_spdlog//:spdlog",
        "@com_github_nlohmann_json//:json",
    ],
)
