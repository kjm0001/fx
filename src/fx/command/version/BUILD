load("//:version.bzl", "FX_VERSION")

cc_library(
    name = "version",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    defines = ["FX_VERSION={0}".format(FX_VERSION)],
    strip_include_prefix = "/src",
    visibility = ["//:__subpackages__"],
    deps = [
        "//src/fx/command/base",
        "//src/fx/result",
        "@com_github_fmtlib_fmt//:fmt",
    ],
)
