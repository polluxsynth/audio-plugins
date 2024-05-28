#!/bin/sh
#
# Find presets that are missing from manifest.ttl
# Run from Presets directory
manifest=manifest.ttl
for i in [A-Z]*ttl; do
  grep $i $manifest > /dev/null || echo "$i not found in $manifest"
done
