#!/bin/sh
# Configura, compila e executa todos os testes.
# Uso: sh scripts/build-e-testes.sh [pasta-de-build]
set -eu

cd "$(dirname "$0")/.."
destino="${1:-build}"

cmake -S . -B "$destino"
cmake --build "$destino" --parallel 2
ctest --test-dir "$destino" --output-on-failure
