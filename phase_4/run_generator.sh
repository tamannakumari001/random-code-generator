#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <seed> [--functions N] [--statements N] [--expr-depth N] [--control-depth N]" >&2
    exit 1
fi

seed="$1"
shift

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source_file="$script_dir/ast.cpp"
binary_file="/tmp/phase4_random_code_generator"
output_file="$script_dir/code.py"

g++ -std=c++17 -Wall -Wextra -pedantic "$source_file" -o "$binary_file"
"$binary_file" --seed "$seed" "$@" > "$output_file"
python3 -m py_compile "$output_file"

echo "Wrote generated code to $output_file" >&2
