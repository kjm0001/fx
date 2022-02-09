proto_library(
    name = "descriptor_proto",
    srcs = glob(["*.proto"]),
    strip_import_prefix = "/src/protobuf",
)

cc_proto_library(
    name = "descriptor_cc_proto",
    visibility = ["//:__subpackages__"],
    deps = [":descriptor_proto"],
)
