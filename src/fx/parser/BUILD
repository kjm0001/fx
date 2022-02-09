cc_library(
    name = "parser",
    srcs = glob(["*.cpp"]),
    hdrs = glob(["*.hpp"]),
    strip_include_prefix = "/src",
    visibility = ["//:__subpackages__"],
    deps = [
        "//src/fx/parser/validator",
        "//src/fx/parser/yaml_to_json",
        "//src/fx/result",
        "//src/protobuf/fx/descriptor/v1beta:descriptor_cc_proto",
        "@com_github_fmtlib_fmt//:fmt",
        "@com_github_jbeder_yaml_cpp//:yaml-cpp",
        "@com_github_nlohmann_json//:json",
        "@com_google_protobuf//:protobuf",
    ],
)
