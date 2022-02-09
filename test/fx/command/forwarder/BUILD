cc_test(
    name = "forwarder",
    size = "small",
    srcs = glob(["*.cpp"]),
    data = glob(["__data__/**/*"]),
    deps = [
        "//src/fx/argparse",
        "//src/fx/command/base",
        "//src/fx/command/forwarder",
        "//src/fx/parser",
        "//src/fx/result",
        "//src/fx/util",
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "//test/helper",
        "@com_github_fmtlib_fmt//:fmt",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest_main",
    ],
)
