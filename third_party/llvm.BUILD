load("@llvm-bazel//:llvm-project-overlay/llvm/template_rule.bzl", "template_rule")

template_rule(
    name = "clang_tidy_config",
    src = "clang-tools-extra/clang-tidy/clang-tidy-config.h.cmake",
    out = "clang-tidy-config.h",
    substitutions = {
        "#cmakedefine01 CLANG_TIDY_ENABLE_STATIC_ANALYZER": "#define CLANG_TIDY_ENABLE_STATIC_ANALYZER 0",
    },
)

cc_binary(
    name = "clang-tidy",
    srcs = [":clang_tidy_config"] + glob([
        "clang-tools-extra/clang-tidy/**/*.cpp",
        "clang-tools-extra/clang-tidy/**/*.h",
    ]),
    stamp = 0,
    visibility = ["//visibility:public"],
    deps = [
        "@llvm-project//clang:analysis",
        "@llvm-project//clang:ast",
        "@llvm-project//clang:ast_matchers",
        "@llvm-project//clang:basic",
        "@llvm-project//clang:config",
        "@llvm-project//clang:format",
        "@llvm-project//clang:frontend",
        "@llvm-project//clang:frontend_rewrite",
        "@llvm-project//clang:lex",
        "@llvm-project//clang:rewrite",
        "@llvm-project//clang:sema",
        "@llvm-project//clang:serialization",
        "@llvm-project//clang:static_analyzer_checkers",
        "@llvm-project//clang:tooling",
        "@llvm-project//clang:tooling_core",
        "@llvm-project//clang:transformer",
        "@llvm-project//llvm:FrontendOpenMP",
        "@llvm-project//llvm:Support",
    ],
)
