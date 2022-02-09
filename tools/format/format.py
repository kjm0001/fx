import sys
import json
import subprocess

def runner(language, commands, is_test):
  print("\nRunning {language} formatter{mode}...\n".format(
    language = language,
    mode = " in test mode" if is_test else ""
  ))

  process = subprocess.run(commands)
  if process.returncode != 0:
    exit(process.returncode)

args = json.loads(sys.argv[1])
languages = args["language"]["value"]
format_all_languages = "all" in languages
is_test_mode = args["test"]["value"]

if format_all_languages or "bazel" in languages:
  target = "buildifier-test" if is_test_mode else "buildifier"
  runner("Bazel", ["bazel", "run", f"//tools/format:{target}"], is_test_mode)

if format_all_languages or "cpp" in languages or "c++" in languages:
  config = "tidy-test" if is_test_mode else "tidy"
  runner("C++", ["bazel", "build", "//...", "--config", config], is_test_mode)
