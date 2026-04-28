#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

PRESET="${1:-libcxx}"
BUILD_DIR="$REPO_ROOT/build/$PRESET"

cmake --preset "$PRESET"
cmake --build --preset "$PRESET" -j

ln -sf "$BUILD_DIR/compile_commands.json" "$REPO_ROOT/compile_commands.json"
echo "compile_commands.json -> build/$PRESET/compile_commands.json"