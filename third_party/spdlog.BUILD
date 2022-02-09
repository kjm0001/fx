cc_library(
    name = "spdlog",
    srcs = glob(
        [
            "include/spdlog/**/*-inl.h",
            "src/**",
        ],
        exclude = ["include/spdlog/fmt/bundled/**"],
    ),
    hdrs = glob(
        ["include/spdlog/**"],
        exclude = [
            "**/*-inl.h",
            "include/spdlog/fmt/bundled/**",
        ],
    ),
    defines = [
        "SPDLOG_COMPILED_LIB",
        # Use @com_github_fmtlib_fmt//:fmt, defined below as a dep
        "SPDLOG_FMT_EXTERNAL",
    ],
    includes = ["include"],
    strip_include_prefix = "include/",
    visibility = ["//visibility:public"],
    deps = ["@com_github_fmtlib_fmt//:fmt"],
)
