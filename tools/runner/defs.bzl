"""Run any executable as a CLI tool with arguments."""

def _runner_impl(ctx):
    ctx.actions.expand_template(
        template = ctx.file._template,
        output = ctx.outputs.executable,
        is_executable = True,
        substitutions = {
            "{RELATIVE_EXECUTABLE_PATH}": ctx.executable.tool.path,
            "{ENVVARS}": " ".join([
                "{key}={value}".format(key = key, value = ctx.attr.envvars[key])
                for key in ctx.attr.envvars
            ]),
        },
    )

    return DefaultInfo(
        executable = ctx.outputs.executable,
        runfiles = ctx.attr.tool.default_runfiles,
    )

runner = rule(
    implementation = _runner_impl,
    executable = True,
    attrs = {
        "_template": attr.label(
            default = "runner.template",
            allow_single_file = True,
        ),
        "tool": attr.label(
            cfg = "host",
            executable = True,
            mandatory = True,
            allow_single_file = True,
        ),
        "envvars": attr.string_dict(
            default = {},
            mandatory = False,
        ),
    },
)
