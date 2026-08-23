#!/bin/sh
# install-macos.sh
#
# Installs the MiMi-d VST3 bundle to the current user's VST3 plugin
# folder and clears the macOS quarantine flag, so it loads without a
# Gatekeeper prompt.
#
# Expects to be run from wherever it was downloaded, sitting alongside
# MiMi-d.vst3-macos.zip. It locates that zip relative to its own
# location so an be invoked from anywhere, e.g:
#   sh ~/Downloads/install-macos.sh
#
# Usage:
#   sh install-macos.sh

set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)

ZIP_NAME="MiMi-d.vst3-macos.zip"
BUNDLE_NAME="MiMi-d.vst3"
ZIP_PATH="$SCRIPT_DIR/$ZIP_NAME"
DEST_DIR="$HOME/Library/Audio/Plug-Ins/VST3"

if [ ! -f "$ZIP_PATH" ]; then
    echo "error: $ZIP_NAME not found next to this script ($SCRIPT_DIR)" >&2
    echo "       (this script must be run from the same folder it was downloaded to)" >&2
    exit 1
fi

mkdir -p "$DEST_DIR"

# Remove any previous install first, so files from an old version's
# bundle that no longer exist in the new build don't linger behind.
if [ -d "$DEST_DIR/$BUNDLE_NAME" ]; then
    echo "Removing existing install ($DEST_DIR/$BUNDLE_NAME)..."
    rm -rf "$DEST_DIR/$BUNDLE_NAME"
fi

echo "Installing $BUNDLE_NAME to $DEST_DIR ..."
unzip -q -o "$ZIP_PATH" -d "$DEST_DIR"

if [ ! -d "$DEST_DIR/$BUNDLE_NAME" ]; then
    echo "error: extraction finished but $BUNDLE_NAME was not found in $DEST_DIR" >&2
    echo "       (check that the zip's top-level entry is actually named $BUNDLE_NAME)" >&2
    exit 1
fi

echo "Clearing quarantine flag..."
xattr -cr "$DEST_DIR/$BUNDLE_NAME"

echo ""
echo "Done. Rescan plugins in your DAW to pick up the new version."
