cc_test(
    name = "yaml_to_json",
    size = "small",
    srcs = glob(["*.cpp"]),
    deps = [
        "//src/fx/parser/yaml_to_json",
        "//src/fx/result",
        "@com_github_jbeder_yaml_cpp//:yaml-cpp",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest_main",
    ],
)
