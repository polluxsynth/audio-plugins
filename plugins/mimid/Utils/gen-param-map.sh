#!/bin/sh
# gen-param-map.sh
#
# Extracts a "symbol -> enum identifier -> default min max value" map
# from ParamDefs.h (X-macros), for use by ttl2data.sh.
#
# Expects lines shaped like:
#   PARAM(PARAMNO, PG, SP, NAME, SYMBOL, MIN, MAX, DEFAULT, SETFUNC)
# e.g.:
#   PARAM(UNISON_PAN, PG_KEYASGN, SP_NONE, "Dual Width", "unisonwidth", 0, 10, 10, setUnisonPanAmt)
#
# Only real invocation lines (leading whitespace then literally "PARAM(")
# are matched, skipping #define and #under etc.
#
# Output, one per line, in the same order the parameters appear in the
# file (which is also DPF's enum assignment order):
#   <symbol> <TAB> <enum-name> <TAB> <default-value>
#
# Caveats:
#   - PARAMNO/SYMBOL/DEFAULT are fields 1/5/8 of a plain comma split, so
#     this will misparse if NAME (or any other field) ever contains a
#     literal comma.
#   - Assumes MIN, MAN and DEFAULT are plain numeric literals (an optional
#     trailing f/F float suffix is stripped automatically). If they are an
#     expression/constant rather than a literal number, a warning is
#     printed and the raw text is emitted as-is.
#
# Usage:
#   gen-param-map.sh ParamDefs.h > params.map

set -eu

if [ $# -ne 1 ]; then
    echo "Usage: $0 <ParamDefs.h>" >&2
    exit 1
fi

FILE=$1

if [ ! -f "$FILE" ]; then
    echo "error: not found: $FILE" >&2
    exit 1
fi

check_literal() {
    local default=$1
    case $default in
        ''|*[!0-9.+-]*)
            echo "warning: non-literal DEFAULT for \"$symbol\": $default -- check manually" >&2
            ;;
    esac
}

strip_f_suffix() {
    local val=$1
    val=$(printf '%s\n' "$val" | sed 's/^\([0-9]*\.\?[0-9]\+\)[fF]$/\1/')
    echo "$val"
}

# One matching line per real PARAM(...) invocation.
grep -E '^[[:blank:]]*PARAM\(' "$FILE" | while IFS= read -r line; do

    # Strip everything up to "PARAM(" and everything from the matching
    # ")" onward, leaving just the comma-separated argument list.
    inner=$(printf '%s\n' "$line" | sed -e 's/^[[:blank:]]*PARAM(//' -e 's/).*$//')

    # Field 1 = PARAMNO
    # Field 5 = SYMBOL
    # Field 6 = MIN
    # Field 7 = MAX
    # Field 8 = DEFAULT.
    paramno=$(printf '%s\n' "$inner" | cut -d, -f1 | sed 's/^[[:blank:]]*//;s/[[:blank:]]*$//')
    symbol=$(printf '%s\n' "$inner"  | cut -d, -f5 | sed 's/^[[:blank:]]*//;s/[[:blank:]]*$//;s/"//g')
    #minval=$(printf '%s\n' "$inner" | cut -d, -f6 | sed 's/^[[:blank:]]*//;s/[[:blank:]]*$//')
    #maxval=$(printf '%s\n' "$inner" | cut -d, -f7 | sed 's/^[[:blank:]]*//;s/[[:blank:]]*$//')
    default=$(printf '%s\n' "$inner" | cut -d, -f8 | sed 's/^[[:blank:]]*//;s/[[:blank:]]*$//')

    if [ -z "$paramno" ] || [ -z "$symbol" ]; then
        echo "warning: could not parse PARAM(...) line: $line" >&2
        continue
    fi

    # Strip a trailing C float suffix (e.g. "0.5f" -> "0.5").
    #minval=$(strip_f_suffix "$minval")
    #maxval=$(strip_f_suffix "$maxval")
    default=$(strip_f_suffix "$default")

    #check_literal "$minval"
    #check_literal "$maxval"
    check_literal "$default"

    printf '%s\t%s\t%s\n' "$symbol" "$paramno" "$default"
done
