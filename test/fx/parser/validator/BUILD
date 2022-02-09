cc_test(
    name = "validator",
    size = "small",
    srcs = glob(["*.cpp"]),
    deps = [
        "//src/fx/parser/validator",
        "//src/fx/result",
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "//test/helper",
        "@com_github_fmtlib_fmt//:fmt",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest_main",
    ],
)
