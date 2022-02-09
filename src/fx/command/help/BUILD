cc_library(
    name = "help",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    strip_include_prefix = "/src",
    visibility = ["//:__subpackages__"],
    deps = [
        "//src/fx/command/base",
        "//src/fx/result",
        "@com_github_fmtlib_fmt//:fmt",
    ],
)
