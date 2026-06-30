#!/usr/bin/env bash
# Regenerate all configured framework headers. This is slow (several minutes).
# Prefer targeted runs instead:
#   cd tools/codegen && PYTHONPATH=. ../../.venv/bin/python3 bean_gen.py --framework foundation
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SDK="${SDKROOT:-$(xcrun --show-sdk-path)}"
PYTHON="${ROOT}/.venv/bin/python3"
CODEGEN="${ROOT}/tools/codegen"

if [[ ! -x "${PYTHON}" ]]; then
  python3 -m venv "${ROOT}/.venv"
  "${ROOT}/.venv/bin/pip" install -q -r "${CODEGEN}/requirements.txt"
fi

cd "${CODEGEN}"
PYTHONPATH=. "${PYTHON}" bean_gen.py --sdk "${SDK}" "$@"
