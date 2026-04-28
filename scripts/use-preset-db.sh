#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

PRESET="${1:?usage: scripts/use-preset-db.sh <libcxx|asan|tsan|release|bench>}"
BUILD_DIR="$REPO_ROOT/build/$PRESET"

if [[ ! -f "$BUILD_DIR/compile_commands.json" ]]; then
  echo "ERROR: missing $BUILD_DIR/compile_commands.json"
  echo "Run: cmake --preset $PRESET"
  exit 1
fi

ln -sf "$BUILD_DIR/compile_commands.json" "$REPO_ROOT/compile_commands.json"
echo "compile_commands.json -> build/$PRESET/compile_commands.json"