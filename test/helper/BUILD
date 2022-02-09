cc_library(
    name = "helper",
    testonly = True,
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    visibility = ["//test:__subpackages__"],
    deps = [
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "@com_github_nlohmann_json//:json",
        "@com_google_googletest//:gtest",
    ],
)
