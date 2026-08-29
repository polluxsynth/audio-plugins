#!/usr/bin/env python3
"""
vstpreset_writer.py

Writes a single binary Steinberg .vstpreset file from a flat list of
(lv2-style symbol, value) pairs, in the exact byte format expected by:

  1. The generic VST3 "Preset File Format" (see Steinberg's
     public.sdk/source/vst/vstpresetfile.h/.cpp):

         0   +---------------------------+
             | HEADER                    |
             | header id ('VST3')        |   4 bytes
             | version                   |   4 bytes (int32 LE)
             | ASCII-encoded class id    |   32 bytes
         +---| offset to chunk list      |   8 bytes (int64 LE)
         |   +---------------------------+
         |   | DATA AREA                 |
         |   |   'Comp' chunk data       |
         |   |   'Cont' chunk data (0 B) |
         |   |   'Info' chunk data (xml) |
         +-->+---------------------------+
             | CHUNK LIST                |
             | list id ('List')          |   4 bytes
             | entry count               |   4 bytes (int32 LE)
             |  per entry:                |
             |   chunk id                |   4 bytes
             |   offset to chunk data    |   8 bytes (int64 LE)
             |   size of chunk data      |   8 bytes (int64 LE)
             +---------------------------+

     The 32-byte class id is the plain uppercase-hex encoding of the
     16 raw bytes of the *component* (processor) class FUID -- exactly
     the form already used for the --component-fuid argument.

  2. DPF's own IComponent::getState()/setState() encoding (see
     DistrhoPluginVST3.cpp), which is what actually goes inside the
     'Comp' chunk:

         "__dpf_program__" <FF> <program-index> <FF>
         "__dpf_parameters_begin__" <FF>
         <symbol> <FF> <value-as-text> <FF>   (repeated, one per parameter)
         "__dpf_parameters_end__" <FF>
         <FE>                                  (terminator, single byte)

     ...with every 0xFF byte then replaced by a NUL (0x00) byte. Values
     are written as plain decimal ASCII text; DPF parses them back with
     atoi()/atof() depending on the parameter's integer hint, and both
     happily parse either "24" or "24.0", so no special float/int
     formatting distinction is required here.

     Only a 'Comp' chunk is required for the plugin to load its state;
     the 'Cont' chunk is included empty (matching what a real host
     produces, since DPF's edit controller reports its own state as
     "not implemented" and delegates entirely to the component state),
     and the 'Info' chunk is included as a small courtesy for hosts
     that show preset metadata (category/vendor/name) in their browser
     -- it is not required for correctness.

This module has no dependencies beyond the Python 3 standard library.
"""

import argparse
import struct
import sys

FORMAT_VERSION = 1
CLASS_ID_SIZE = 32


def build_comp_chunk(params, program_index=0):
    """params: list of (symbol, value_text) pairs, in plugin parameter order."""
    pieces = []
    pieces.append("__dpf_program__\xff%d\xff" % program_index)
    pieces.append("__dpf_parameters_begin__\xff")
    for symbol, value_text in params:
        pieces.append("%s\xff%s\xff" % (symbol, value_text))
    pieces.append("__dpf_parameters_end__\xff")
    text = "".join(pieces) + "\xfe"

    # Build the raw byte stream: 0xFF -> 0x00, everything else stays as
    # its own single byte (all content here is plain ASCII, so this is
    # a 1:1 codepoint->byte mapping other than the 0xFF placeholder).
    out = bytearray()
    for ch in text:
        code = ord(ch)
        if code == 0xFF:
            out.append(0x00)
        else:
            out.append(code)
    return bytes(out)


def build_info_chunk(plugin_name, vendor, category):
    xml = (
        '<?xml version="1.0" encoding="utf-8"?>\n'
        '<MetaInfo>\n'
        '<Attribute id="MediaType" value="VstPreset" type="string" flags="writeProtected"/>\n'
        '<Attribute id="PlugInCategory" value="%s" type="string" flags="writeProtected"/>\n'
        '<Attribute id="PlugInName" value="%s" type="string" flags="writeProtected"/>\n'
        '<Attribute id="PlugInVendor" value="%s" type="string" flags="writeProtected"/>\n'
        '</MetaInfo>\n'
    ) % (category, plugin_name, vendor)
    return xml.encode("utf-8")


def write_vstpreset(out_path, component_fuid_hex, params, program_index=0,
                     plugin_name="", vendor="", category="Instrument",
                     write_info=True):
    component_fuid_hex = component_fuid_hex.strip().upper()
    if len(component_fuid_hex) != CLASS_ID_SIZE or \
       any(c not in "0123456789ABCDEF" for c in component_fuid_hex):
        raise ValueError(
            "component FUID must be exactly %d uppercase hex characters, got %r"
            % (CLASS_ID_SIZE, component_fuid_hex))

    comp_data = build_comp_chunk(params, program_index=program_index)
    cont_data = b""  # DPF's controller reports V3_NOT_IMPLEMENTED -> 0-byte chunk
    info_data = build_info_chunk(plugin_name, vendor, category) if write_info else None

    with open(out_path, "wb") as f:
        # ---- header ----
        f.write(b"VST3")
        f.write(struct.pack("<i", FORMAT_VERSION))
        f.write(component_fuid_hex.encode("ascii"))
        list_offset_pos = f.tell()
        f.write(struct.pack("<q", 0))  # placeholder, patched at the end

        entries = []  # (chunk_id bytes[4], offset, size)

        offset = f.tell()
        f.write(comp_data)
        entries.append((b"Comp", offset, len(comp_data)))

        offset = f.tell()
        f.write(cont_data)
        entries.append((b"Cont", offset, len(cont_data)))

        if info_data is not None:
            offset = f.tell()
            f.write(info_data)
            entries.append((b"Info", offset, len(info_data)))

        # ---- chunk list ----
        list_pos = f.tell()
        f.write(b"List")
        f.write(struct.pack("<i", len(entries)))
        for chunk_id, chunk_offset, chunk_size in entries:
            f.write(chunk_id)
            f.write(struct.pack("<q", chunk_offset))
            f.write(struct.pack("<q", chunk_size))

        # ---- patch the list offset back in the header ----
        f.seek(list_offset_pos)
        f.write(struct.pack("<q", list_pos))


def parse_intermediate(fh):
    """Reads the simple intermediate format produced by ttl2vstpreset.sh:

         LABEL<TAB><preset label>
         PARAM<TAB><symbol><TAB><value>
         PARAM<TAB><symbol><TAB><value>
         ...
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
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--component-fuid", required=True,
                     help="32 hex-char processor/component class FUID")
    ap.add_argument("--input", default="-",
                     help="intermediate LABEL/PARAM file (default: stdin)")
    ap.add_argument("--output", required=True,
                     help="path to write the .vstpreset file to")
    ap.add_argument("--program-index", type=int, default=0,
                     help="value stored in the __dpf_program__ slot (default: 0)")
    ap.add_argument("--plugin-name", default="")
    ap.add_argument("--vendor", default="")
    ap.add_argument("--category", default="Instrument")
    ap.add_argument("--no-info-chunk", action="store_true",
                     help="omit the optional 'Info' metadata chunk")
    args = ap.parse_args()

    if args.input == "-":
        _, params = parse_intermediate(sys.stdin)
    else:
        with open(args.input, "r", encoding="utf-8") as fh:
            _, params = parse_intermediate(fh)

    write_vstpreset(
        args.output,
        args.component_fuid,
        params,
        program_index=args.program_index,
        plugin_name=args.plugin_name,
        vendor=args.vendor,
        category=args.category,
        write_info=not args.no_info_chunk,
    )


if __name__ == "__main__":
    main()
