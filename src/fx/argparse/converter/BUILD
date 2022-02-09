cc_library(
    name = "converter",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    strip_include_prefix = "/src",
    visibility = ["//:__subpackages__"],
    deps = [
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "@com_github_bfgroup_lyra//:lyra",
        "@com_github_nlohmann_json//:json",
    ],
)
