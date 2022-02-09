import json
import sys
import os

args = json.loads(sys.argv[1])

print("args:")
for name, result in args.items():
  value = result["value"]
  print(f"  {name}={value}")

print("envars:")
for key, value in sorted(os.environ.items()):
    print(f"  {key}={value}")
