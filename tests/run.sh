#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
mkdir -p bin
CXX="${CXX:-g++}"
"$CXX" -std=c++17 -I.. -o bin/schema_check schema_check.cpp
exec bin/schema_check
