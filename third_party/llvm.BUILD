load("@llvm-raw//utils/bazel/llvm-project-overlay/llvm:template_rule.bzl", "template_rule")

template_rule(
    name = "clang_tidy_config",
    src = "clang-tools-extra/clang-tidy/clang-tidy-config.h.cmake",
    out = "clang-tidy-config.h",
    substitutions = {
        "#cmakedefine01 CLANG_TIDY_ENABLE_STATIC_ANALYZER": "#define CLANG_TIDY_ENABLE_STATIC_ANALYZER 0",
    },
)

cc_binary(
    name = "clang_tidy_make_confusable_table",
    srcs = ["clang-tools-extra/clang-tidy/misc/ConfusableTable/BuildConfusableTable.cpp"],
    deps = [
        "@llvm-project//llvm:Support",
    ],
)

genrule(
    name = "clang_tidy_confusables",
    srcs = ["clang-tools-extra/clang-tidy/misc/ConfusableTable/confusables.txt"],
    outs = ["Confusables.inc"],
    cmd = "$(location :clang_tidy_make_confusable_table) $(SRCS) $(OUTS)",
    exec_tools = [":clang_tidy_make_confusable_table"],
)

cc_binary(
    name = "clang_tidy",
    srcs = [
        ":clang_tidy_config",
        ":clang_tidy_confusables",
    ] + glob(
        [
            "clang-tools-extra/clang-tidy/**/*.cpp",
            "clang-tools-extra/clang-tidy/**/*.h",
        ],
        exclude = ["clang-tools-extra/clang-tidy/misc/ConfusableTable/**/*"],
    ),
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
