cc_library(
    name = "dispatcher",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    strip_include_prefix = "/src",
    visibility = ["//:__subpackages__"],
    deps = [
        "//src/fx/command/base",
        "//src/fx/command/forwarder",
        "//src/fx/command/help",
        "//src/fx/command/list",
        "//src/fx/command/version",
        "@com_github_fmtlib_fmt//:fmt",
        "@com_github_gabime_spdlog//:spdlog",
    ],
)
