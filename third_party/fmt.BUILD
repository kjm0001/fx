cc_library(
    name = "fmt",
    hdrs = glob(["include/fmt/*.h"]),
    defines = [
        "FMT_HEADER_ONLY",
        "FMT_NO_FMT_STRING_ALIAS",
    ],
    includes = ["include"],
    strip_include_prefix = "include/",
    visibility = ["//visibility:public"],
)
