#!/bin/sh
# ttl2vstpreset.sh
#
# Converts a directory of LV2 preset .ttl files (one preset per file,
# standard LV2 Presets extension layout) into binary Steinberg
# .vstpreset files for the VST3 build of a DPF-based plugin.
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
# IComponent::getState()/setState() round-trip always produces.
#
# Usage:
#   ttl2vstpreset.sh <presets-dir> <output-dir> [<param-map-file>]
#
# <param-map-file> format: see gen-param-map.sh.
# (Tab separated lv2 parameter name, source code symbol, default value)
#
# Requires python3 (only used for the binary chunk/header assembly;
# see vstpreset_writer.py, which must be next to this script).

set -eu

SELF_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
WRITER="$SELF_DIR/vstpreset_writer.py"

# FUID of the Audio Module Class (processor/component). Per the VST3
# preset file format, a .vstpreset's header classID is always the
# *component* FUID, never the controller's, even though both are
# compiled into the plugin.
COMPONENT_FUID="2046504473616C63644D694D00000000"

PLUGIN_NAME="MiMi-d"
PLUGIN_VENDOR="Pollux"
PLUGIN_CATEGORY="Instrument"

if [ $# -lt 2 ] || [ $# -gt 3 ]; then
    echo "Usage: $0 <presets-dir> <output-dir> [param-map-file]" >&2
    exit 1
fi

PRESET_DIR=$1
OUT_DIR=$2
PARAM_MAP=${3:-}

if [ ! -d "$PRESET_DIR" ]; then
    echo "error: not a directory: $PRESET_DIR" >&2
    exit 1
fi
if [ -n "$PARAM_MAP" ] && [ ! -f "$PARAM_MAP" ]; then
    echo "error: param map not found: $PARAM_MAP" >&2
    exit 1
fi
if [ ! -f "$WRITER" ]; then
    echo "error: helper not found next to this script: $WRITER" >&2
    exit 1
fi
if ! command -v python3 >/dev/null 2>&1; then
    echo "error: python3 is required (for binary .vstpreset assembly) but was not found" >&2
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

# --- Pass 2: turn each intermediate file into a .vstpreset ---

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

    out="$OUT_DIR/$safe.vstpreset"

    # Avoid clobbering same-named presets from different source files.
    if [ -e "$out" ]; then
        i=2
        while [ -e "$OUT_DIR/$safe ($i).vstpreset" ]; do
            i=$((i + 1))
        done
        out="$OUT_DIR/$safe ($i).vstpreset"
    fi

    python3 "$WRITER" \
        --component-fuid "$COMPONENT_FUID" \
        --plugin-name "$PLUGIN_NAME" \
        --vendor "$PLUGIN_VENDOR" \
        --category "$PLUGIN_CATEGORY" \
        --input "$f" \
        --output "$out"

    echo "wrote: $out" >&2
    count=$((count + 1))
done

if [ "$count" -eq 0 ]; then
    echo "error: no presets were converted" >&2
    exit 1
fi

echo "Converted $count preset(s) into $OUT_DIR" >&2
