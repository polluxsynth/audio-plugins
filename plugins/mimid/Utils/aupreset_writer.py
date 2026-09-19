#!/usr/bin/env python3
"""
aupreset_writer.py

Writes a single .aupreset file (a standard Apple CFPropertyList, XML
plist on disk) from a flat list of (lv2-style symbol, value) pairs,
matching the exact dictionary shape DPF's AU backend builds in
retrieveClassInfo() / expects back in restoreClassInfo() (see
DistrhoPluginAU.cpp):

    {
        version      = 0                     # SInt32
        type         = <4-char OSType>       # SInt32, packed big-endian
        subtype      = <4-char OSType>       # SInt32, packed big-endian
        manufacturer = <4-char OSType>       # SInt32, packed big-endian
        name         = "<preset label>"
        data = {
            params = [
                { "<symbol>" = <Float32 value> },
                ...
            ]
        }
    }

The four-char codes must match the plugin's compiled-in identity:
    type          DISTRHO_PLUGIN_AU_TYPE   (e.g. "aumu" for a synth)
    subtype       DISTRHO_PLUGIN_UNIQUE_ID
    manufacturer  DISTRHO_PLUGIN_BRAND_ID

Unlike .vstpreset, DPF's AU state has no "program" or "states" slots
here: those only appear when DISTRHO_PLUGIN_WANT_PROGRAMS /
DISTRHO_PLUGIN_WANT_STATE are defined, and MiMi-d defines neither, so
only "params" is ever written. --program-index is still accepted, for
a consistent command-line shape with vstpreset_writer.py, but it is
otherwise unused here.

This module only depends on the Python 3 standard library (plistlib
handles the property-list serialization itself), and has no
dependency on vstpreset_writer.py -- the two writers are independent
scripts that happen to read the same simple intermediate format.
"""

import argparse
import plistlib
import struct
import sys

VERSION = 0


def pack_four_char_code(code):
    """Packs a 4-character code into the big-endian 32-bit integer
    form DPF's d_cconst() and AU's OSType both use, e.g. "aumu" ->
    (a<<24)|(u<<16)|(m<<8)|u."""
    code_bytes = code.encode("ascii")
    if len(code_bytes) != 4:
        raise ValueError(
            "four-char code must be exactly 4 ASCII characters, got %r"
            % code)
    return int.from_bytes(code_bytes, byteorder="big", signed=True)


def build_params_array(params):
    """params: list of (symbol, value_text) pairs, in plugin
    parameter order. Returns a list of single-key dicts, matching the
    array-of-single-entry-dicts shape DPF's AU backend produces."""
    entries = []
    for symbol, value_text in params:
        entries.append({symbol: float(value_text)})
    return entries


def write_aupreset(out_path, au_type, au_subtype, au_manufacturer,
                    preset_name, params):
    plist = {
        "version": VERSION,
        "type": pack_four_char_code(au_type),
        "subtype": pack_four_char_code(au_subtype),
        "manufacturer": pack_four_char_code(au_manufacturer),
        "name": preset_name,
        "data": {
            "params": build_params_array(params),
        },
    }

    with open(out_path, "wb") as f:
        plistlib.dump(plist, f, fmt=plistlib.FMT_XML)


def parse_intermediate(fh):
    """Reads the simple intermediate format produced by ttl2preset.sh:

         LABEL<TAB><preset label>
         PARAM<TAB><symbol><TAB><value>
         PARAM<TAB><symbol><TAB><value>
         ...

    (Tabs can't appear within a field: the label has any literal tabs
    stripped when it's written, and LV2 symbols are restricted by the
    LV2 spec to [a-zA-Z_][a-zA-Z0-9_]*, which excludes tabs anyway --
    so plain split("\t") is safe here without any escaping scheme.)
    """
    label = None
    params = []
    for line in fh:
        line = line.rstrip("\n")
        if not line:
            continue
        fields = line.split("\t")
        if fields[0] == "LABEL" and len(fields) >= 2:
            label = fields[1]
        elif fields[0] == "PARAM" and len(fields) >= 3:
            params.append((fields[1], fields[2]))
        else:
            print("warning: ignoring malformed intermediate line: %r" % line,
                  file=sys.stderr)
    if label is None:
        raise ValueError("intermediate input had no LABEL line")
    return label, params


def main():
    ap = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--au-type", required=True,
                     help="4-char AU type code, e.g. 'aumu' "
                          "(DISTRHO_PLUGIN_AU_TYPE)")
    ap.add_argument("--au-subtype", required=True,
                     help="4-char AU subtype code "
                          "(DISTRHO_PLUGIN_UNIQUE_ID)")
    ap.add_argument("--au-manufacturer", required=True,
                     help="4-char AU manufacturer code "
                          "(DISTRHO_PLUGIN_BRAND_ID)")
    ap.add_argument("--preset-name", default=None,
                     help="preset display name embedded in the plist "
                          "(default: the LABEL from --input)")
    ap.add_argument("--input", default="-",
                     help="intermediate LABEL/PARAM file "
                          "(default: stdin)")
    ap.add_argument("--output", required=True,
                     help="path to write the .aupreset file to")
    ap.add_argument("--program-index", type=int, default=0,
                     help="accepted for command-line parity with "
                          "vstpreset_writer.py; DPF's AU 'program' "
                          "slot only exists when the plugin defines "
                          "DISTRHO_PLUGIN_WANT_PROGRAMS, so this is "
                          "otherwise ignored")
    args = ap.parse_args()

    if args.input == "-":
        label, params = parse_intermediate(sys.stdin)
    else:
        with open(args.input, "r", encoding="utf-8") as fh:
            label, params = parse_intermediate(fh)

    preset_name = args.preset_name if args.preset_name is not None \
        else label

    write_aupreset(
        args.output,
        args.au_type,
        args.au_subtype,
        args.au_manufacturer,
        preset_name,
        params,
    )


if __name__ == "__main__":
    main()
