"""Run clang-format and clang-tidy on cc targets."""

load("@bazel_tools//tools/cpp:toolchain_utils.bzl", "find_cpp_toolchain")
load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "CPP_COMPILE_ACTION_NAME")

# Tag for cc_library to ignore during tidying.
CLANG_TIDY_IGNORE = "clang-tidy-ignore"

def _unique(files):
    seen = {}
    for file in files:
        if file not in seen:
            seen[file] = True
    return [file for file in seen]

def _run_tidy(ctx, compiler_flags, compilation_context, source):
    tidier = ctx.actions.declare_file(
        "_{0}.tidier.sh".format(source.path),
    )
    result = ctx.actions.declare_file(
        "_{0}.clang-tidy.yaml".format(source.path),
    )

    ctx.actions.expand_template(
        template = ctx.file._tidy_template,
        output = tidier,
        is_executable = True,
        substitutions = {
            "{RELATIVE_CLANG_TIDY_PATH}": ctx.executable._clang_tidy.path,
            "{RELATIVE_RESULT_PATH}": result.path,
        },
    )

    args = ctx.actions.args()
    args.add("--config-file", ctx.file._tidy_config.path)
    args.add("--export-fixes", result.path)
    args.add("--use-color")
    args.add("--quiet")
    args.add(source.path)
    args.add("--")

    # HACK HACK HACK: for some reason, explicitly including subfolder causes
    # builds to fail. Why? I have no idea — and too lazy to figure it out.
    # Why include the headers? https://chromium.googlesource.com/chromium/src/+/60.0.3082.3/docs/clang_tidy.md
    args.add_all(
        _unique([
            header.dirname
            for header in ctx.files._clang_headers
            if header.dirname.endswith("include")
        ]),
        before_each = "-I",
    )
    args.add_all(compiler_flags)
    args.add_all(compilation_context.defines.to_list(), before_each = "-D")
    args.add_all(
        compilation_context.local_defines.to_list(),
        before_each = "-D",
    )
    args.add_all(
        compilation_context.framework_includes.to_list(),
        before_each = "-F",
    )
    args.add_all(
        compilation_context.includes.to_list(),
        before_each = "-I",
    )
    args.add_all(
        compilation_context.quote_includes.to_list(),
        before_each = "-iquote",
    )
    args.add_all(
        compilation_context.system_includes.to_list(),
        before_each = "-isystem",
    )

    ctx.actions.run(
        inputs = depset(
            direct = [tidier, source, ctx.file._tidy_config] + ctx.files._clang_headers,
            transitive = [compilation_context.headers],
        ),
        outputs = [result],
        tools = [ctx.executable._clang_tidy],
        executable = tidier,
        arguments = [args],
        mnemonic = "ClangTidy",
        progress_message = "Tidying {0}".format(source.short_path),
        use_default_shell_env = True,
        execution_requirements = {
            "no-sandbox": "1",
        },
    )

    return result

def _run_format(ctx, source):
    formatter = ctx.actions.declare_file(
        "_{0}.formatter.sh".format(source.path),
    )
    result = ctx.actions.declare_file("_{0}.format.result".format(source.path))

    ctx.actions.expand_template(
        template = ctx.file._format_template,
        output = formatter,
        is_executable = True,
        substitutions = {
            "{RELATIVE_CLANG_FORMAT_PATH}": ctx.executable._clang_format.path,
            "{RELATIVE_SOURCE_PATH}": source.short_path,
            "{RELATIVE_RESULT_PATH}": result.path,
            "{LINT_MODE}": ctx.attr.lint_mode,
        },
    )

    ctx.actions.run(
        inputs = depset(direct = [formatter, source, ctx.file._format_config]),
        outputs = [result],
        tools = [ctx.executable._clang_format],
        executable = formatter,
        mnemonic = "ClangFormat",
        progress_message = "Formatting {0}".format(source.short_path),
        use_default_shell_env = True,
        execution_requirements = {
            "no-sandbox": "1",
        },
    )

    return result

def _compiler_flags(ctx):
    cc_toolchain = find_cpp_toolchain(ctx)

    feature_configuration = cc_common.configure_features(
        ctx = ctx,
        cc_toolchain = cc_toolchain,
    )
    compile_variables = cc_common.create_compile_variables(
        feature_configuration = feature_configuration,
        cc_toolchain = cc_toolchain,
        user_compile_flags = ctx.fragments.cpp.cxxopts + ctx.fragments.cpp.copts + ctx.fragments.cpp.linkopts,
    )
    toolchain_flags = cc_common.get_memory_inefficient_command_line(
        feature_configuration = feature_configuration,
        action_name = CPP_COMPILE_ACTION_NAME,
        variables = compile_variables,
    )
    rule_flags = ctx.rule.attr.copts if hasattr(ctx.rule.attr, "copts") else []

    return toolchain_flags + rule_flags

def _rule_sources(ctx):
    sources = ctx.rule.files.srcs if hasattr(ctx.rule.attr, "srcs") else []
    headers = ctx.rule.files.hdrs if hasattr(ctx.rule.attr, "hdrs") else []
    return [sources, headers]

def _tidy_aspect_impl(target, ctx):
    if CLANG_TIDY_IGNORE in ctx.rule.attr.tags:
        return []

    sources, headers = _rule_sources(ctx)
    compiler_flags = _compiler_flags(ctx)
    compilation_context = target[CcInfo].compilation_context

    reports = []
    for source in sources:
        reports.append(_run_format(ctx, source))
        reports.append(
            _run_tidy(
                ctx,
                compiler_flags,
                compilation_context,
                source,
            ),
        )

    # TODO(jathu): Also run clang-tidy on headers. For now, this only runs
    # formatting on headers. Running clang-tidy seems to cause
    # recursive/conflicting errors.
    for header in headers:
        reports.append(_run_format(ctx, header))

    return [OutputGroupInfo(report = depset(direct = reports))]

def _create_aspect(lint_mode):
    return aspect(
        implementation = _tidy_aspect_impl,
        fragments = ["cpp"],
        attr_aspects = ["deps"],
        required_providers = [CcInfo],
        attrs = {
            "_cc_toolchain": attr.label(
                default = Label("@bazel_tools//tools/cpp:current_cc_toolchain"),
            ),
            "_clang_headers": attr.label(
                default = Label("@llvm-project//clang:builtin_headers_gen"),
            ),
            "_clang_tidy": attr.label(
                default = Label("@llvm-project-raw//:clang-tidy"),
                cfg = "host",
                executable = True,
                allow_single_file = True,
            ),
            "_tidy_template": attr.label(
                default = "tidy.template",
                allow_single_file = True,
            ),
            "_tidy_config": attr.label(
                default = Label("//:.clang-tidy"),
                allow_single_file = True,
            ),
            "_clang_format": attr.label(
                default = Label("@llvm-project//clang:clang-format"),
                cfg = "host",
                executable = True,
                allow_single_file = True,
            ),
            "_format_template": attr.label(
                default = "format.template",
                allow_single_file = True,
            ),
            "_format_config": attr.label(
                default = Label("//:.clang-format"),
                allow_single_file = True,
            ),
            "lint_mode": attr.string(
                default = lint_mode,
                values = ["overwrite", "check"],
            ),
        },
    )

# Exports ----------------------------------------------------------------------

tidy_aspect = _create_aspect("overwrite")

tidy_aspect_check = _create_aspect("check")
