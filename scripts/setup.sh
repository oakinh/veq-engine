#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

echo "==> Configuring default VEQ dev preset"
cmake --preset libcxx

echo "==> Building"
cmake --build --preset libcxx -j

if [[ -f "$REPO_ROOT/build/libcxx/compile_commands.json" ]]; then
  ln -sf "$REPO_ROOT/build/libcxx/compile_commands.json" "$REPO_ROOT/compile_commands.json"
  echo "compile_commands.json -> build/libcxx/compile_commands.json"
fi

echo "==> Done"