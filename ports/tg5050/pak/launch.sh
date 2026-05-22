#!/bin/sh
set -eu

PAK_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
APP_ID="theythemerollin"

BIN="${PAK_DIR}/bin/theythemerollin"

if [ -x "${BIN}" ]; then
  export THEYTHEMEROLLIN_PAK_DIR="${PAK_DIR}"
  export THEYTHEMEROLLIN_LOG_DIR="${USERDATA_PATH}/../tg5050/logs"

  mkdir -p "$(dirname "${THEYTHEMEROLLIN_LOG_DIR}")" 2>/dev/null || true

  cd "${PAK_DIR}"
  exec "${BIN}"
else
  echo "Executable not found: ${BIN}"
  exit 0
fi
