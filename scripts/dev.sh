#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

PRESET="${1:-libcxx}"
BUILD_DIR="$REPO_ROOT/build/$PRESET"
SCRATCH_BIN="$BUILD_DIR/veq_scratch"

echo "==> Configuring ($PRESET preset)"
cmake --preset "$PRESET"

if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
  ln -sf "$BUILD_DIR/compile_commands.json" "$REPO_ROOT/compile_commands.json"
fi

echo "==> Building"
cmake --build --preset "$PRESET" -j

if [[ ! -x "$SCRATCH_BIN" ]]; then
  echo "ERROR: scratch binary not found: $SCRATCH_BIN"
  exit 1
fi

echo "==> Running scratch"
"$SCRATCH_BIN" "$@"