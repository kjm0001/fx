load("//:version.bzl", "FX_VERSION")

cc_binary(
    name = "main",
    srcs = ["main.cpp"],
    defines = ["FX_VERSION={0}".format(FX_VERSION)],
    visibility = ["//:__pkg__"],
    deps = [
        "//src/fx/dispatcher",
        "@com_github_fmtlib_fmt//:fmt",
        "@com_github_gabime_spdlog//:spdlog",
    ],
)

cc_binary(
    name = "version_emitter",
    srcs = ["version_emitter.cpp"],
    defines = ["FX_VERSION={0}".format(FX_VERSION)],
)
