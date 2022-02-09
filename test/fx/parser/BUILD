cc_test(
    name = "parser",
    size = "small",
    srcs = glob(["*.cpp"]),
    data = glob(["__data__/**/*"]),
    deps = [
        "//src/fx/parser",
        "//src/fx/result",
        "@com_github_jbeder_yaml_cpp//:yaml-cpp",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest_main",
        "@com_google_protobuf//:protobuf",
    ],
)
