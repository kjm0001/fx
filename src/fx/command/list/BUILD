load("//:version.bzl", "FX_VERSION")

cc_library(
    name = "list",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    defines = ["FX_VERSION={0}".format(FX_VERSION)],
    strip_include_prefix = "/src",
    visibility = ["//:__subpackages__"],
    deps = [
        "//src/fx/command/base",
        "//src/fx/parser",
        "//src/fx/result",
        "//src/fx/util",
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "@com_github_fmtlib_fmt//:fmt",
        "@com_github_gabime_spdlog//:spdlog",
    ],
)
