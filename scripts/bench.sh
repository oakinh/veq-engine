#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

PRESET="bench"
BUILD_DIR="$REPO_ROOT/build/$PRESET"
BENCH_BIN="$BUILD_DIR/benchmarks/veq_bench"

echo "==> Configuring ($PRESET preset)"
cmake --preset "$PRESET"

if [[ -f "$BUILD_DIR/compile_commands.json" ]]; then
  ln -sf "$BUILD_DIR/compile_commands.json" "$REPO_ROOT/compile_commands.json"
fi

echo "==> Building"
cmake --build --preset "$PRESET" -j

if [[ ! -x "$BENCH_BIN" ]]; then
  echo "ERROR: benchmark binary not found: $BENCH_BIN"
  echo "Hint: make sure benchmarks/CMakeLists.txt creates a target/binary named veq_bench."
  exit 1
fi

echo "==> Running benchmarks"
exec "$BENCH_BIN" "$@"