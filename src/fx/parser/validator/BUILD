cc_library(
    name = "validator",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    strip_include_prefix = "/src",
    visibility = ["//:__subpackages__"],
    deps = [
        "//src/fx/result",
        "//src/fx/util",
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "@com_github_fmtlib_fmt//:fmt",
    ],
)
