cc_test(
    name = "argparse",
    size = "small",
    srcs = glob(["*.cpp"]),
    deps = [
        "//src/fx/argparse",
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "//test/helper",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)
