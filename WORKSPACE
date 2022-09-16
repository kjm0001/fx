workspace(name = "fx")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "io_bazel_rules_go",
    sha256 = "2b1641428dff9018f9e85c0384f03ec6c10660d935b750e3fa1492a281a53b0f",
    urls = [
        "https://github.com/bazelbuild/rules_go/releases/download/v0.29.0/rules_go-v0.29.0.zip",
    ],
)

load("@io_bazel_rules_go//go:deps.bzl", "go_register_toolchains", "go_rules_dependencies")

go_rules_dependencies()

go_register_toolchains(version = "1.17.2")

http_archive(
    name = "bazel_gazelle",
    sha256 = "de69a09dc70417580aabf20a28619bb3ef60d038470c7cf8442fafcf627c21cb",
    urls = [
        "https://github.com/bazelbuild/bazel-gazelle/releases/download/v0.24.0/bazel-gazelle-v0.24.0.tar.gz",
    ],
)

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")

gazelle_dependencies()

http_archive(
    name = "com_google_protobuf",
    sha256 = "87407cd28e7a9c95d9f61a098a53cf031109d451a7763e7dd1253abf8b4df422",
    strip_prefix = "protobuf-3.19.1",
    urls = [
        "https://github.com/protocolbuffers/protobuf/archive/v3.19.1.tar.gz",
    ],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

http_archive(
    name = "rules_pkg",
    sha256 = "62eeb544ff1ef41d786e329e1536c1d541bb9bcad27ae984d57f18f314018e66",
    urls = [
        "https://github.com/bazelbuild/rules_pkg/releases/download/0.6.0/rules_pkg-0.6.0.tar.gz",
    ],
)

load("@rules_pkg//:deps.bzl", "rules_pkg_dependencies")

rules_pkg_dependencies()

LLVM_VERSION = "llvmorg-15.0.0"

http_archive(
    name = "llvm-raw",
    build_file = "//third_party:llvm.BUILD",
    sha256 = "36d83cd84e1caf2bcfda1669c029e2b949adb9860cff01e7d3246ac2348b11ae",
    strip_prefix = "llvm-project-{0}".format(LLVM_VERSION),
    url = "https://github.com/llvm/llvm-project/archive/refs/tags/{0}.tar.gz".format(LLVM_VERSION),
)

load("@llvm-raw//utils/bazel:configure.bzl", "llvm_configure", "llvm_disable_optional_support_deps")

llvm_configure(name = "llvm-project")

llvm_disable_optional_support_deps()

http_archive(
    name = "com_github_bazelbuild_buildtools",
    sha256 = "ae34c344514e08c23e90da0e2d6cb700fcd28e80c02e23e4d5715dddcb42f7b3",
    strip_prefix = "buildtools-4.2.2",
    urls = [
        "https://github.com/bazelbuild/buildtools/archive/refs/tags/4.2.2.tar.gz",
    ],
)

http_archive(
    name = "com_github_gabime_spdlog",
    build_file = "//third_party:spdlog.BUILD",
    sha256 = "6fff9215f5cb81760be4cc16d033526d1080427d236e86d70bb02994f85e3d38",
    strip_prefix = "spdlog-1.9.2",
    urls = ["https://github.com/gabime/spdlog/archive/v1.9.2.tar.gz"],
)

http_archive(
    name = "com_github_jbeder_yaml_cpp",
    sha256 = "43e6a9fcb146ad871515f0d0873947e5d497a1c9c60c58cb102a97b47208b7c3",
    strip_prefix = "yaml-cpp-yaml-cpp-0.7.0",
    urls = ["https://github.com/jbeder/yaml-cpp/archive/yaml-cpp-0.7.0.tar.gz"],
)

http_archive(
    name = "com_github_nelhage_rules_boost",
    sha256 = "c3264642c6f77a894c19432fed9b0c0d1ad156b56f8e32c13abac4c682bd0873",
    strip_prefix = "rules_boost-c8b9b4a75c4301778d2e256b8d72ce47a6c9a1a4",
    urls = ["https://github.com/nelhage/rules_boost/archive/c8b9b4a75c4301778d2e256b8d72ce47a6c9a1a4.tar.gz"],
)

load("@com_github_nelhage_rules_boost//:boost/boost.bzl", "boost_deps")

boost_deps()

http_archive(
    name = "com_github_nlohmann_json",
    build_file = "//third_party:json.BUILD",
    sha256 = "ea4b0084709fb934f92ca0a68669daa0fe6f2a2c6400bf353454993a834bb0bb",
    strip_prefix = "json-3.10.5",
    urls = ["https://github.com/nlohmann/json/archive/refs/tags/v3.10.5.zip"],
)

http_archive(
    name = "com_github_bfgroup_lyra",
    build_file = "//third_party:lyra.BUILD",
    sha256 = "e27c6eca98dad626bd17c236aea57cc8ab8e72dea0c66e140d0ce18740ba4d5b",
    strip_prefix = "Lyra-1.6",
    urls = ["https://github.com/bfgroup/Lyra/archive/refs/tags/1.6.zip"],
)

http_archive(
    name = "com_google_googletest",
    sha256 = "353571c2440176ded91c2de6d6cd88ddd41401d14692ec1f99e35d013feda55a",
    strip_prefix = "googletest-release-1.11.0",
    urls = ["https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip"],
)

http_archive(
    name = "com_github_fmtlib_fmt",
    build_file = "//third_party:fmt.BUILD",
    sha256 = "f2aad1a340d27c0b22cf23268c1cbdffb8472a242b95daf2d2311eed8d4948fc",
    strip_prefix = "fmt-8.1.1",
    urls = ["https://github.com/fmtlib/fmt/archive/refs/tags/8.1.1.zip"],
)
