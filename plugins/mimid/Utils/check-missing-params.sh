#!/bin/sh
# check-missing-params.sh
#
# Reports, for every *.ttl preset file in <presets-dir>, which
# parameter symbols declared in <ParamDefs.h> that file does not
# already contain. Read-only -- never modifies any preset file.
#
# Requires gen-param-map.sh to be present alongside this script (used
# purely to get the full list of current parameter symbols out of
# ParamDefs.h -- that's the only awk usage in this whole pipeline).
# Everything else here is grep/sed/sort/comm.
#
# Assumes each preset's parameters appear one per line as
# `lv2:symbol "name" ;` -- true of every preset file addparam.sh
# produces/maintains. If some file ever wraps a symbol across multiple
# lines, that entry would be missed by this scan.
#
# Usage:
#   check-missing-params.sh <ParamDefs.h> <presets-dir>

set -eu

if [ $# -ne 2 ]; then
    echo "Usage: $0 <ParamDefs.h> <presets-dir>" >&2
    exit 1
fi

PARAMDEFS=$1
PRESET_DIR=$2
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
GEN_PARAM_MAP="$SCRIPT_DIR/gen-param-map.sh"

if [ ! -f "$PARAMDEFS" ]; then
    echo "error: not found: $PARAMDEFS" >&2
    exit 1
fi
if [ ! -d "$PRESET_DIR" ]; then
    echo "error: not a directory: $PRESET_DIR" >&2
    exit 1
fi
if [ ! -f "$GEN_PARAM_MAP" ]; then
    echo "error: gen-param-map.sh not found next to this script ($SCRIPT_DIR)" >&2
    exit 1
fi

# Full symbol list, one per line, sorted -- required for comm below.
ALL_SYMBOLS=$(sh "$GEN_PARAM_MAP" "$PARAMDEFS" | cut -f1 | sort)

any_missing=0
TMP_PRESENT=$(mktemp)
trap 'rm -f "$TMP_PRESENT"' EXIT INT TERM

find "$PRESET_DIR" -maxdepth 1 -type f -name '*.ttl' | sort | while IFS= read -r file; do

    grep -o 'lv2:symbol[[:blank:]]*"[^"]*"' "$file" \
        | sed 's/.*"\([^"]*\)"/\1/' \
        | sort -u > "$TMP_PRESENT"

    # Lines in ALL_SYMBOLS that do NOT appear (as a whole line) in
    # TMP_PRESENT -- i.e. symbols this file is missing.
    missing=$(printf '%s\n' "$ALL_SYMBOLS" | grep -Fxvf "$TMP_PRESENT" || true)

    if [ -n "$missing" ]; then
        echo "$file"
        printf '%s\n' "$missing" | sed 's/^/    /'
    fi
done
