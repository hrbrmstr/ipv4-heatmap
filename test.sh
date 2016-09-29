#!/bin/bash
cat extra/ip1m | bin/ipv4-heatmap -o bin/out.png
RES=$?
exit $RES