load("@com_github_bazelbuild_buildtools//buildifier:def.bzl", "buildifier")

exports_files([
    "format.template",
    "tidy.template",
])

# Format all Bazel files.
#   Usage: bazel run //tools/format:buildifier
buildifier(
    name = "buildifier",
    lint_mode = "fix",
)

# Test if Bazel files are formatted.
#   Usage: bazel run //tools/format:buildifier-test
buildifier(
    name = "buildifier-test",
    mode = "check",
)
