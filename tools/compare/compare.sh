#!/bin/bash

set -e

current_branch=$(git rev-parse --abbrev-ref HEAD)
echo -e "Comparing ${current_branch}..."
exec open "https://github.com/jathu/fx/compare/${current_branch}"
