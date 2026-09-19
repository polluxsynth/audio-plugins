#!/bin/sh
# ttl2preset.sh
#
# Converts a directory of LV2 preset .ttl files (one preset per file,
# standard LV2 Presets extension layout) into the native preset
# format(s) of other plugin builds of the same DPF-based plugin:
# binary Steinberg .vstpreset files for VST3, and/or .aupreset
# CFPropertyList files for AU.
#
# Assumes each preset file contains blocks shaped like:
#
#   <...> a pset:Preset ;
#       rdfs:label "Warm Pad" ;
#       lv2:appliesTo <...> ;
#       lv2:port [
#           lv2:symbol "cutoff" ;
#           pset:value 0.75
#       ] , [
#           lv2:symbol "resonance" ;
#           pset:value 0.30
#       ] .
#
# This is a pragmatic line-scanner, not a real Turtle parser. It is
# tolerant of arbitrary line breaks/indentation within a file, but it
# assumes:
#   - rdfs:label appears exactly once, as a plain double-quoted string
#     with no escaped quotes inside it
#   - every relevant parameter is expressed as lv2:symbol "..." followed
#     (anywhere later in the file) by pset:value <number>
#   - numeric values are plain decimal/exponential (no LV2 special forms)
#
# Any parameter a given preset file doesn't specify is filled in from that
# parameter's coded DEFAULT (taken from the param-map file, see below),
# so every generated program is always complete, since that's what DPF's own
# state save/restore round-trip always produces for either format.
#
# This is a single driver over two independent writer scripts
# (vstpreset_writer.py, aupreset_writer.py). Both consume the same
# intermediate LABEL/PARAM representation of a preset (built once in
# Pass 1 below, shared between formats) and each serializes it into
# whatever its own target format needs -- a hand-packed binary blob
# for .vstpreset, a CFPropertyList for .aupreset. Which formats get
# written is controlled by --format; everything about *where* the
# resulting files end up on disk (e.g. the .aupreset install path
# convention) is intentionally left up to the caller/installer, not
# handled here.
#
# Usage:
#   ttl2preset.sh [options] <presets-dir> <output-dir> [<param-map-file>]
#
# Options:
#   --format FORMAT       vst3, au, or both (default: both)
#   --fuid FUID           VST3 component FUID to embed in the header
#                         (see the COMPONENT_FUID comment below for
#                         what this must be). Overrides the coded
#                         default. Ignored unless --format includes vst3.
#   --name NAME           Plugin name (VST3 Info chunk only).
#                         Overrides the coded default.
#   --vendor VENDOR       Plugin vendor (VST3 Info chunk only).
#                         Overrides the coded default.
#   --category CATEGORY   Plugin category (VST3 Info chunk only).
#                         Overrides the coded default.
#   --au-type CODE         4-char AU type code (DISTRHO_PLUGIN_AU_TYPE).
#                         Overrides the coded default. Ignored unless
#                         --format includes au.
#   --au-subtype CODE      4-char AU subtype code
#                         (DISTRHO_PLUGIN_UNIQUE_ID). Overrides the
#                         coded default. Ignored unless --format
#                         includes au.
#   --au-manufacturer CODE 4-char AU manufacturer code
#                         (DISTRHO_PLUGIN_BRAND_ID). Overrides the
#                         coded default. Ignored unless --format
#                         includes au.
#
# <param-map-file> format: see gen-param-map.sh.
# (Tab separated lv2 parameter name, source code symbol, default value)
#
# Requires python3 (only used for the binary/plist assembly; see
# vstpreset_writer.py and aupreset_writer.py, which are independent
# scripts and must both be next to this script).

set -eu

SELF_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
VST3_WRITER="$SELF_DIR/vstpreset_writer.py"
AU_WRITER="$SELF_DIR/aupreset_writer.py"

# FUID of the Audio Module Class (processor/component). Per the VST3
# preset file format, a .vstpreset's header classID is always the
# *component* FUID, never the controller's, even though both are
# compiled into the plugin.
#
# These are the coded defaults; each can be overridden on the command
# line (--fuid, --name, --vendor, --category, --au-type, --au-subtype,
# --au-manufacturer) without editing the script, e.g. when reusing it
# for a different DPF plugin build.
COMPONENT_FUID="2046504473616C63644D694D00000000"

PLUGIN_NAME="MiMi-d"
PLUGIN_VENDOR="Pollux"
PLUGIN_CATEGORY="Instrument"

# AU identity codes. These must match DISTRHO_PLUGIN_AU_TYPE,
# DISTRHO_PLUGIN_UNIQUE_ID and DISTRHO_PLUGIN_BRAND_ID in
# DistrhoPluginInfo.h exactly, or a host will not recognize the
# preset as belonging to this plugin.
AU_TYPE="aumu"
AU_SUBTYPE="MiMd"
AU_MANUFACTURER="Pllx"

FORMAT="both"

usage() {
    echo "Usage: $0 [options] <presets-dir> <output-dir> [param-map-file]" >&2
    echo "Options:" >&2
    echo "  --format FORMAT         vst3, au, or both (default: $FORMAT)" >&2
    echo "  --fuid FUID             VST3 component FUID (default: $COMPONENT_FUID)" >&2
    echo "  --name NAME             plugin name, VST3 only (default: $PLUGIN_NAME)" >&2
    echo "  --vendor VENDOR         plugin vendor, VST3 only (default: $PLUGIN_VENDOR)" >&2
    echo "  --category CATEGORY     plugin category, VST3 only (default: $PLUGIN_CATEGORY)" >&2
    echo "  --au-type CODE          AU type code (default: $AU_TYPE)" >&2
    echo "  --au-subtype CODE       AU subtype code (default: $AU_SUBTYPE)" >&2
    echo "  --au-manufacturer CODE  AU manufacturer code (default: $AU_MANUFACTURER)" >&2
}

# --- parse options, then positional args ---
#
# Manual parsing rather than getopts, since getopts (per POSIX) only
# handles single-character options and these are long options.

POSITIONAL=""
npos=0
while [ $# -gt 0 ]; do
    case "$1" in
        --format)
            [ $# -ge 2 ] || { echo "error: --format requires an argument" >&2; exit 1; }
            FORMAT=$2
            shift 2
            ;;
        --fuid)
            [ $# -ge 2 ] || { echo "error: --fuid requires an argument" >&2; exit 1; }
            COMPONENT_FUID=$2
            shift 2
            ;;
        --name)
            [ $# -ge 2 ] || { echo "error: --name requires an argument" >&2; exit 1; }
            PLUGIN_NAME=$2
            shift 2
            ;;
        --vendor)
            [ $# -ge 2 ] || { echo "error: --vendor requires an argument" >&2; exit 1; }
            PLUGIN_VENDOR=$2
            shift 2
            ;;
        --category)
            [ $# -ge 2 ] || { echo "error: --category requires an argument" >&2; exit 1; }
            PLUGIN_CATEGORY=$2
            shift 2
            ;;
        --au-type)
            [ $# -ge 2 ] || { echo "error: --au-type requires an argument" >&2; exit 1; }
            AU_TYPE=$2
            shift 2
            ;;
        --au-subtype)
            [ $# -ge 2 ] || { echo "error: --au-subtype requires an argument" >&2; exit 1; }
            AU_SUBTYPE=$2
            shift 2
            ;;
        --au-manufacturer)
            [ $# -ge 2 ] || { echo "error: --au-manufacturer requires an argument" >&2; exit 1; }
            AU_MANUFACTURER=$2
            shift 2
            ;;
        --format=*|--fuid=*|--name=*|--vendor=*|--category=*|\
        --au-type=*|--au-subtype=*|--au-manufacturer=*)
            opt=${1%%=*}
            val=${1#*=}
            case "$opt" in
                --format) FORMAT=$val ;;
                --fuid) COMPONENT_FUID=$val ;;
                --name) PLUGIN_NAME=$val ;;
                --vendor) PLUGIN_VENDOR=$val ;;
                --category) PLUGIN_CATEGORY=$val ;;
                --au-type) AU_TYPE=$val ;;
                --au-subtype) AU_SUBTYPE=$val ;;
                --au-manufacturer) AU_MANUFACTURER=$val ;;
            esac
            shift
            ;;
        --help|-h)
            usage
            exit 0
            ;;
        --)
            shift
            while [ $# -gt 0 ]; do
                npos=$((npos + 1))
                eval "POS_$npos=\$1"
                shift
            done
            ;;
        -*)
            echo "error: unknown option: $1" >&2
            usage
            exit 1
            ;;
        *)
            npos=$((npos + 1))
            eval "POS_$npos=\$1"
            shift
            ;;
    esac
done

if [ "$npos" -lt 2 ] || [ "$npos" -gt 3 ]; then
    usage
    exit 1
fi

case "$FORMAT" in
    vst3|au|both) ;;
    *)
        echo "error: --format must be vst3, au, or both (got: $FORMAT)" >&2
        exit 1
        ;;
esac

PRESET_DIR=$POS_1
OUT_DIR=$POS_2
PARAM_MAP=${POS_3:-}

if [ ! -d "$PRESET_DIR" ]; then
    echo "error: not a directory: $PRESET_DIR" >&2
    exit 1
fi
if [ -n "$PARAM_MAP" ] && [ ! -f "$PARAM_MAP" ]; then
    echo "error: param map not found: $PARAM_MAP" >&2
    exit 1
fi
if [ "$FORMAT" = "vst3" ] || [ "$FORMAT" = "both" ]; then
    if [ ! -f "$VST3_WRITER" ]; then
        echo "error: helper not found next to this script: $VST3_WRITER" >&2
        exit 1
    fi
fi
if [ "$FORMAT" = "au" ] || [ "$FORMAT" = "both" ]; then
    if [ ! -f "$AU_WRITER" ]; then
        echo "error: helper not found next to this script: $AU_WRITER" >&2
        exit 1
    fi
fi
if ! command -v python3 >/dev/null 2>&1; then
    echo "error: python3 is required (for binary/plist assembly) but was not found" >&2
    exit 1
fi

mkdir -p "$OUT_DIR"

# manifest.ttl is the LV2 bundle manifest, not a preset, so skip it.
FILELIST=$(find "$PRESET_DIR" -maxdepth 1 -type f -name '*.ttl' \
    ! -name 'manifest.ttl' | sort)

if [ -z "$FILELIST" ]; then
    echo "error: no .ttl files found in $PRESET_DIR" >&2
    exit 1
fi

WORKDIR=$(mktemp -d)
trap 'rm -rf "$WORKDIR"' EXIT INT TERM

# --- Pass 1: for each .ttl, emit an intermediate LABEL/PARAM file ---
#
# One intermediate file per preset, named after its source .ttl (e.g.
# "$WORKDIR/Basic_Saw.params" for "Basic_Saw.ttl") so a failure in
# pass 2 is easy to trace back to its source file. Each contains a
# LABEL line followed by one PARAM line per (symbol, value) pair found
# in the .ttl, in the order encountered.
#
# This pass is entirely format-agnostic -- it knows nothing about
# .vstpreset or .aupreset -- so it is shared unchanged between both
# output formats.

echo "$FILELIST" | awk -v mapfile="$PARAM_MAP" -v workdir="$WORKDIR" '
BEGIN {
    haveMap = (mapfile != "")
    mapCount = 0
    if (haveMap) {
        while ((getline line < mapfile) > 0) {
            if (line == "" || line ~ /^#/) continue
            nf = split(line, f, /\t/)
            if (nf < 1 || f[1] == "") continue
            if (!(f[1] in known)) {
                mapCount++
                mapOrder[mapCount] = f[1]
            }
            known[f[1]] = 1
        }
        close(mapfile)
    }
}

function slurp(path,    line, out) {
    out = ""
    while ((getline line < path) > 0) {
        out = out " " line
    }
    close(path)
    return out
}

function extract_quoted(s, from,    rest, q1, rest2, q2, val) {
    rest = substr(s, from)
    q1 = index(rest, "\"")
    if (q1 == 0) { g_after = length(s) + 1; return "" }
    rest2 = substr(rest, q1 + 1)
    q2 = index(rest2, "\"")
    if (q2 == 0) { g_after = length(s) + 1; return "" }
    val = substr(rest2, 1, q2 - 1)
    g_after = from + q1 + q2
    return val
}

{
    path = $0

    # basename of the source .ttl, minus its extension. Used both as
    # the fallback label (below) and, unchanged, as the name of the
    # intermediate file for this preset -- since .ttl filenames in a
    # single directory are already unique and filesystem-safe, there
    # is no need to invent a synthetic name, and reusing the source
    # basename makes it obvious which intermediate file came from
    # which preset when debugging.
    split(path, pparts, "/")
    basename = pparts[length(pparts)]
    sub(/\.ttl$/, "", basename)

    content = slurp(path)

    # --- extract rdfs:label "..." ---
    lidx = index(content, "rdfs:label")
    label = ""
    if (lidx > 0) {
        label = extract_quoted(content, lidx)
    }
    if (label == "") {
        label = basename
        print "warning: no rdfs:label found in " path ", using filename" > "/dev/stderr"
    }

    # The intermediate LABEL/PARAM format is tab-delimited with no
    # escaping, so a literal tab embedded in a rdfs:label (unusual,
    # but not forbidden by Turtle string syntax) would corrupt the
    # line. Flatten any such tab to a space rather than adding an
    # escaping scheme for what should be a one-in-a-million case.
    if (label ~ /\t/) {
        print "warning: rdfs:label in " path " contains a tab; replacing with a space" > "/dev/stderr"
        gsub(/\t/, " ", label)
    }

    # --- extract every (lv2:symbol "sym", pset:value NUM) pair, in order ---
    delete provided
    delete symOrder
    nsym = 0
    work = content
    pos = 1
    while (1) {
        sidx = index(substr(work, pos), "lv2:symbol")
        if (sidx == 0) break
        sidx += pos - 1
        sym = extract_quoted(work, sidx)
        after_sym = g_after

        vidx = index(substr(work, after_sym), "pset:value")
        if (vidx == 0) {
            print "warning: lv2:symbol \"" sym "\" with no following pset:value in " path > "/dev/stderr"
            break
        }
        vidx += after_sym - 1 + length("pset:value")

        rest = substr(work, vidx)
        if (match(rest, /^[ \t]*[-+]?[0-9]+(\.[0-9]+)?([eE][-+]?[0-9]+)?/)) {
            numstr = substr(rest, RSTART, RLENGTH)
            gsub(/[ \t]/, "", numstr)
        } else {
            print "warning: could not parse numeric value after pset:value in " path > "/dev/stderr"
            numstr = "0"
        }

        if (haveMap && !(sym in known)) {
            print "warning: unknown parameter symbol \"" sym "\" in " path > "/dev/stderr"
        }

        if (!(sym in provided)) {
            nsym++
            symOrder[nsym] = sym
        }
        provided[sym] = numstr

        pos = vidx + length(numstr)
    }

    if (haveMap) {
        for (m = 1; m <= mapCount; m++) {
            if (!(mapOrder[m] in provided))
                print "warning: known parameter \"" mapOrder[m] "\" missing from " path > "/dev/stderr"
        }
    }

    outfile = workdir "/" basename ".params"
    printf "LABEL\t%s\n", label > outfile
    for (i = 1; i <= nsym; i++) {
        sym = symOrder[i]
        printf "PARAM\t%s\t%s\n", sym, provided[sym] >> outfile
    }
    close(outfile)
}
'

# --- Pass 2: turn each intermediate file into one file per requested format ---

count=0
for f in "$WORKDIR"/*.params; do
    [ -e "$f" ] || continue

    label=$(head -n1 "$f" | cut -f2-)

    # Sanitize the label into a filesystem-safe basename: replace path
    # separators (which cannot appear in a rdfs:label anyway) with
    # '-', and trim leading/trailing whitespace. Everything else
    # (spaces, punctuation) is left as-is.
    safe=$(printf '%s' "$label" | sed -e 's#[/\\]#-#g' -e 's/^[[:space:]]*//' -e 's/[[:space:]]*$//')
    if [ -z "$safe" ]; then
        safe="Untitled"
    fi

    wrote_any=0

    if [ "$FORMAT" = "vst3" ] || [ "$FORMAT" = "both" ]; then
        out="$OUT_DIR/$safe.vstpreset"
        if [ -e "$out" ]; then
            i=2
            while [ -e "$OUT_DIR/$safe ($i).vstpreset" ]; do
                i=$((i + 1))
            done
            out="$OUT_DIR/$safe ($i).vstpreset"
        fi

        python3 "$VST3_WRITER" \
            --component-fuid "$COMPONENT_FUID" \
            --plugin-name "$PLUGIN_NAME" \
            --vendor "$PLUGIN_VENDOR" \
            --category "$PLUGIN_CATEGORY" \
            --input "$f" \
            --output "$out"

        echo "wrote: $out" >&2
        wrote_any=1
    fi

    if [ "$FORMAT" = "au" ] || [ "$FORMAT" = "both" ]; then
        out="$OUT_DIR/$safe.aupreset"
        if [ -e "$out" ]; then
            i=2
            while [ -e "$OUT_DIR/$safe ($i).aupreset" ]; do
                i=$((i + 1))
            done
            out="$OUT_DIR/$safe ($i).aupreset"
        fi

        python3 "$AU_WRITER" \
            --au-type "$AU_TYPE" \
            --au-subtype "$AU_SUBTYPE" \
            --au-manufacturer "$AU_MANUFACTURER" \
            --input "$f" \
            --output "$out"

        echo "wrote: $out" >&2
        wrote_any=1
    fi

    [ "$wrote_any" -eq 1 ] && count=$((count + 1))
done

if [ "$count" -eq 0 ]; then
    echo "error: no presets were converted" >&2
    exit 1
fi

echo "Converted $count preset(s) (format: $FORMAT) into $OUT_DIR" >&2
