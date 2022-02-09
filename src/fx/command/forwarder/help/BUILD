cc_library(
    name = "help",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    strip_include_prefix = "/src",
    visibility = ["//:__subpackages__"],
    deps = [
        "//src/fx/result",
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "@boost//:algorithm",
        "@boost//:regex",
        "@com_github_fmtlib_fmt//:fmt",
        "@com_github_gabime_spdlog//:spdlog",
    ],
)
