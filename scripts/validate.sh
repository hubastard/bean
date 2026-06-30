#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SDK="${SDKROOT:-$(xcrun --show-sdk-path)}"
INCLUDE="${ROOT}/include"
OUT="${ROOT}/build/validate"
PYTHON="${ROOT}/.venv/bin/python3"
CODEGEN="${ROOT}/tools/codegen"

mkdir -p "${OUT}"

echo "SDK: ${SDK}"
xcrun clang --version | head -1

CFLAGS=(
  -std=c99
  -Wall
  -Wextra
  -Werror
  -I"${INCLUDE}"
  -isysroot "${SDK}"
  -iframework "${SDK}/System/Library/Frameworks"
  -F "${SDK}/System/Library/Frameworks"
)

link_frameworks() {
  local out="$1"
  shift
  xcrun clang "${CFLAGS[@]}" "$@" -framework Foundation -framework AppKit -framework CoreGraphics -lobjc -o "${out}"
}

echo "Running apinotes unit tests..."
cd "${CODEGEN}"
PYTHONPATH=. "${PYTHON}" test_apinotes.py

echo "Running docs unit tests..."
PYTHONPATH=. "${PYTHON}" test_docs.py

echo "Compiling foundation smoke test..."
link_frameworks "${OUT}/test_foundation" "${ROOT}/tests/smoke/test_foundation.c"

echo "Compiling appkit smoke test..."
link_frameworks "${OUT}/test_appkit" "${ROOT}/tests/smoke/test_appkit.c"

echo "Compiling hello_window example..."
link_frameworks "${OUT}/hello_window" "${ROOT}/examples/hello_window/main.c"

echo "Syntax-checking generated framework headers..."
FRAMEWORKS="$(
  cd "${CODEGEN}"
  PYTHONPATH=. "${PYTHON}" -c "from bean.config import GENERATION_ORDER; print(' '.join(GENERATION_ORDER))"
)"
HEADER_FAILURES=()
for fw in ${FRAMEWORKS}; do
  header="${INCLUDE}/bean/${fw}.h"
  if [[ ! -f "${header}" ]]; then
    echo "Skipping bean/${fw}.h (not generated)"
    continue
  fi
  echo "Checking bean/${fw}.h..."
  if ! xcrun clang "${CFLAGS[@]}" -fsyntax-only -x c -include "bean/${fw}.h" /dev/null \
    2>"${OUT}/${fw}.header.errors"; then
    HEADER_FAILURES+=("${fw}")
    sed -n '1,8p' "${OUT}/${fw}.header.errors" >&2
  fi
done

if ((${#HEADER_FAILURES[@]} > 0)); then
  echo "Header syntax check failed for: ${HEADER_FAILURES[*]}" >&2
  exit 1
fi

echo "Validation build succeeded."
