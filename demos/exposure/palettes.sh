#!/usr/bin/env bash
#
# Generate examples using forward and inverse palettes

for f in brbg puor rdbu rdgy rdylbu spectral bupu reds ylgnbu ylorbr ylorrd viridis ; do

  echo "Generating $f..."
  ipv4-heatmap -P $f -o $f.png < ~/data/study.ips

  echo "Generating $f-inverted..."
  ipv4-heatmap -i -P $f -o $f-inverted.png < ~/data/study.ips

done
