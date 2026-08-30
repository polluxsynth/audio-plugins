#!/usr/bin/env python3
"""
dpf_vst3_fuid.py

Reproduces, byte-for-byte, the VST3 class IDs (FUIDs) that DPF's
distrho/src/DistrhoPluginVST3.cpp generates for a plugin, purely from
static analysis of DistrhoPluginInfo.h (and, if needed, the plugin's
main .cpp). No DPF checkout or build is required to run this script;
it just encodes the same arithmetic DPF itself uses.

Background (verified by reading DPF's actual source, not from memory):

  distrho/src/DistrhoPluginVST3.cpp defines five 16-byte TUIDs, each
  built as four native uint32_t words:

      dpf_tuid_class      = { 'DPF ', 'clas', 0, brand }
      dpf_tuid_component  = { 'DPF ', 'comp', 0, brand }
      dpf_tuid_controller = { 'DPF ', 'ctrl', 0, brand }
      dpf_tuid_processor  = { 'DPF ', 'proc', 0, brand }
      dpf_tuid_view       = { 'DPF ', 'view', 0, brand }

  where each 4-character tag is packed big-endian via d_cconst() (see
  distrho/DistrhoUtils.hpp: (a<<24)|(b<<16)|(c<<8)|d), and 'brand' is
  0 unless DISTRHO_PLUGIN_BRAND_ID is defined (in which case it's that
  macro's 4 characters, packed the same way).

  At plugin-info init time DPF overwrites word index 2 of ALL FIVE
  tuids with plugin->getUniqueId() (truncated to 32 bits):

      dpf_tuid_class[2] = dpf_tuid_component[2] = dpf_tuid_controller[2]
          = dpf_tuid_processor[2] = dpf_tuid_view[2] = sPlugin->getUniqueId();

  getUniqueId() itself defaults (distrho/DistrhoPlugin.hpp) to
  d_cconst(STRINGIFY(DISTRHO_PLUGIN_UNIQUE_ID)) if that macro is
  defined, otherwise it's whatever the plugin overrides it to return
  (for MiMi-d: `return d_cconst('M','i','M','d');` in MiMi-d.cpp).

  Of these five, only dpf_tuid_class is ever registered with the host
  as an actual VST3 class (IPluginFactory::get_class_info, idx 0,
  category "Audio Module Class" -- see DistrhoPluginVST3.cpp around
  create_instance()/get_class_info()). That is the FUID that ends up
  in a .vstpreset's ClassID field. The other four are only used
  internally by DPF/travesty as private query_interface tags and
  never appear in factory metadata or in a preset file.

  IMPORTANT PLATFORM WRINKLE (verified against the actual VST3 SDK,
  steinbergmedia/vst3_pluginterfaces, base/funknown.cpp):

  A .vstpreset's ClassID header field is produced by calling
  FUID::toString() on the class id (public.sdk/source/vst/vstpresetfile.cpp,
  PresetFile::writeClassID -> classID.toString(classString)). FUID::toString()
  has two totally different code paths depending on the COM_COMPATIBLE
  macro, which is 1 on Windows and 0 on Linux/macOS:

    - Linux/macOS (COM_COMPATIBLE=0): a straight hex dump of all 16
      raw bytes, MSB-first per byte, in array order. No reordering.

    - Windows (COM_COMPATIBLE=1): the first 8 bytes are reinterpreted
      as a Windows GUID {DWORD Data1; WORD Data2; WORD Data3;} in
      *native* (little-endian) order and then printed as
      "%08X%04X%04X", which is NOT the same as a plain byte dump of
      those 8 bytes. The last 8 bytes are unaffected (plain dump both
      ways).

  DPF's own TUIDs are plain uint32_t[4] in native memory order, not
  built via the SDK's INLINE_UID()/GUID macros, so this is not a
  hypothetical: the same DPF-built plugin binary produces a DIFFERENT
  ClassID string in a .vstpreset saved by a Windows host than in one
  saved by a Linux/macOS host, for the first 8 bytes. (The last 8
  bytes -- unique id + brand -- always match, since those come out
  the same on both paths.)

  This script prints both encodings so you can pick the one matching
  whichever platform the presets need to load correctly on. If you
  only care about Linux/Carla/REAPER-on-Linux/Bitwig-on-Linux etc.,
  use --platform linux (the default). For presets meant to be dropped
  into a Windows host's preset folder, use --platform windows.

Usage:
    python3 dpf_vst3_fuid.py path/to/DistrhoPluginInfo.h [path/to/Plugin.cpp]
    python3 dpf_vst3_fuid.py path/to/DistrhoPluginInfo.h [path/to/Plugin.cpp] --platform windows
    python3 dpf_vst3_fuid.py path/to/DistrhoPluginInfo.h [path/to/Plugin.cpp] --all
"""

import argparse
import re
import struct
import sys


def d_cconst4(chars: str) -> int:
    """Reproduce DistrhoUtils.hpp's d_cconst(): pack 4 chars big-endian."""
    if len(chars) != 4:
        raise ValueError(f"d_cconst needs exactly 4 characters, got {chars!r}")
    a, b, c, d = (ord(ch) for ch in chars)
    return ((a << 24) | (b << 16) | (c << 8) | d) & 0xFFFFFFFF


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.DOTALL)
    text = re.sub(r"//.*", "", text)
    return text


def find_define(text: str, name: str):
    """Find #define NAME VALUE (VALUE may be a quoted string or a bare token)."""
    m = re.search(rf'#\s*define\s+{re.escape(name)}\s+"?([^\s"]+)"?', text)
    return m.group(1) if m else None


def get_unique_id(info_h_text: str, plugin_cpp_text: str | None) -> int:
    """
    Mirrors DPF's own resolution order:
      1. If DISTRHO_PLUGIN_UNIQUE_ID is #defined, getUniqueId() defaults to
         d_cconst(STRINGIFY(DISTRHO_PLUGIN_UNIQUE_ID)).
      2. Otherwise the plugin must override getUniqueId() itself, almost
         always as `return d_cconst('a','b','c','d');`. Look for that in
         the plugin's main .cpp.
    """
    macro_val = find_define(info_h_text, "DISTRHO_PLUGIN_UNIQUE_ID")
    if macro_val:
        if len(macro_val) != 4:
            raise ValueError(
                f"DISTRHO_PLUGIN_UNIQUE_ID must be exactly 4 characters, "
                f"got {macro_val!r}"
            )
        return d_cconst4(macro_val)

    if plugin_cpp_text is None:
        raise ValueError(
            "DISTRHO_PLUGIN_UNIQUE_ID is not defined in DistrhoPluginInfo.h; "
            "pass the plugin's main .cpp (the one with the getUniqueId() "
            "override) as a second argument."
        )

    # Matches: return d_cconst('M','i','M','d');  (whitespace-tolerant)
    m = re.search(
        r"getUniqueId\s*\([^)]*\)\s*const\s*(?:override\s*)?\{"
        r"\s*return\s+d_cconst\s*\(\s*"
        r"'(.)'\s*,\s*'(.)'\s*,\s*'(.)'\s*,\s*'(.)'\s*\)\s*;",
        plugin_cpp_text,
        flags=re.DOTALL,
    )
    if not m:
        raise ValueError(
            "Could not find a `return d_cconst('a','b','c','d');` inside "
            "a getUniqueId() override in the given plugin .cpp. If the "
            "plugin computes its unique id some other way, this script "
            "can't infer it automatically -- pass --unique-id abcd instead."
        )
    return d_cconst4("".join(m.groups()))


def get_brand_id(info_h_text: str) -> int:
    macro_val = find_define(info_h_text, "DISTRHO_PLUGIN_BRAND_ID")
    if not macro_val:
        return 0  # DPF_VST3_DONT_USE_BRAND_ID behaviour / macro simply absent
    if len(macro_val) != 4:
        raise ValueError(
            f"DISTRHO_PLUGIN_BRAND_ID must be exactly 4 characters, "
            f"got {macro_val!r}"
        )
    return d_cconst4(macro_val)


TAGS = {
    "class": "clas",  # <-- the one that matters: registered "Audio Module Class"
    "component": "comp",  # internal only
    "controller": "ctrl",  # registered "Component Controller Class" (separate-controller builds)
    "processor": "proc",  # internal only
    "view": "view",  # internal only
}

ENTRY = d_cconst4("DPF ")


def build_tuid(tag: str, unique_id: int, brand_id: int) -> bytes:
    word0 = ENTRY
    word1 = d_cconst4(tag)
    word2 = unique_id & 0xFFFFFFFF
    word3 = brand_id & 0xFFFFFFFF
    # Native little-endian packing: this is what memcpy'ing a uint32_t[4]
    # into a char[16] produces on every mainstream DPF build target
    # (x86, x86_64, arm, arm64 are all little-endian).
    return struct.pack("<IIII", word0, word1, word2, word3)


def fuid_tostring_linux(raw16: bytes) -> str:
    """FUID::toString() on Linux/macOS (COM_COMPATIBLE == 0): plain dump."""
    assert len(raw16) == 16
    return raw16.hex().upper()


def fuid_tostring_windows(raw16: bytes) -> str:
    """FUID::toString() on Windows (COM_COMPATIBLE == 1): GUID-style reorder."""
    assert len(raw16) == 16
    data1 = struct.unpack_from("<I", raw16, 0)[0]
    data2 = struct.unpack_from("<H", raw16, 4)[0]
    data3 = struct.unpack_from("<H", raw16, 6)[0]
    tail = raw16[8:16].hex().upper()
    return f"{data1:08X}{data2:04X}{data3:04X}{tail}"


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                  formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("info_header", help="path to DistrhoPluginInfo.h")
    ap.add_argument("plugin_cpp", nargs="?", default=None,
                     help="path to the plugin's main .cpp (needed only if "
                          "DISTRHO_PLUGIN_UNIQUE_ID isn't #defined)")
    ap.add_argument("--unique-id", metavar="ABCD",
                     help="override: 4-character unique id, skips source scanning")
    ap.add_argument("--platform", choices=["linux", "windows", "both"],
                     default="linux",
                     help="which FUID::toString() byte layout to print "
                          "(default: linux; 'both' also shows the Windows form)")
    ap.add_argument("--all", action="store_true",
                     help="also print the four internal-only TUIDs "
                          "(component/controller/processor/view), for "
                          "cross-checking against the travesty-based verifier")
    ap.add_argument("--oneline", action="store_true",
                     help="print only the bare FUID string(s), one per line, "
                          "with no labels/headers -- for capturing directly "
                          "into a shell variable, e.g. "
                          "fuid=$(python3 dpf_vst3_fuid.py ... --oneline). "
                          "Illegal with --platform both, since that produces "
                          "two different strings for the same FUID.")
    ap.add_argument("--raw", action="store_true",
                     help="with --oneline, print the raw byte dump instead "
                          "of the FUID::toString() form. Only permitted "
                          "together with --oneline. Note this is identical "
                          "to the plain --oneline output on --platform linux, "
                          "since FUID::toString() is a plain byte dump there "
                          "-- it only differs from --platform windows.")
    args = ap.parse_args()

    if args.raw and not args.oneline:
        ap.error("--raw is only permitted together with --oneline")
    if args.oneline and args.platform == "both":
        ap.error("--oneline is illegal with --platform both "
                  "(pick --platform linux or --platform windows)")
    if args.oneline and args.all:
        ap.error("--oneline is illegal with --all "
                  "(--oneline only prints the single class FUID)")

    with open(args.info_header, encoding="utf-8") as f:
        info_h_text = strip_comments(f.read())

    plugin_cpp_text = None
    if args.plugin_cpp:
        with open(args.plugin_cpp, encoding="utf-8") as f:
            plugin_cpp_text = strip_comments(f.read())

    if args.unique_id:
        if len(args.unique_id) != 4:
            sys.exit("--unique-id must be exactly 4 characters")
        unique_id = d_cconst4(args.unique_id)
    else:
        unique_id = get_unique_id(info_h_text, plugin_cpp_text)

    brand_id = get_brand_id(info_h_text)

    tags = TAGS if args.all else {"class": TAGS["class"]}

    if args.oneline:
        # Bare value(s) only -- safe to capture with $(...) in a shell script.
        for name, tag in tags.items():
            raw = build_tuid(tag, unique_id, brand_id)
            if args.raw:
                value = raw.hex()
            elif args.platform == "windows":
                value = fuid_tostring_windows(raw)
            else:
                value = fuid_tostring_linux(raw)
            print(value)
        return

    print(f"unique id : 0x{unique_id:08X}")
    print(f"brand id  : 0x{brand_id:08X}"
          + ("  (DISTRHO_PLUGIN_BRAND_ID not set)" if brand_id == 0 else ""))
    print()

    for name, tag in tags.items():
        raw = build_tuid(tag, unique_id, brand_id)
        linux_hex = fuid_tostring_linux(raw)
        windows_hex = fuid_tostring_windows(raw)
        marker = "  <-- use this one for .vstpreset ClassID" if name == "class" else ""
        print(f"{name}{marker}")
        print(f"  raw bytes : {raw.hex()}")
        if args.platform in ("linux", "both"):
            print(f"  linux/mac : {linux_hex}")
        if args.platform in ("windows", "both"):
            print(f"  windows   : {windows_hex}")
        print()


if __name__ == "__main__":
    main()
