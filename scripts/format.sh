#!/usr/bin/env bash
set -euo pipefail

find include src tests benchmarks -type f \( \
    -name "*.hpp" -o \
    -name "*.cpp" -o \
    -name "*.h" -o \
    -name "*.cc" \
\) -print0 | xargs -0 clang-format-19 -i