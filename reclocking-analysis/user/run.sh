#!/bin/bash

SECTION=$1
BINARY=./reclocking-analysis
AVX_ARGS=$(objdump -h $BINARY | grep -e "$SECTION\s" | sed "s/^ [0-9]\+\s\+\.[a-zA-Z_0-9]\+\s\+\([0-9a-f]\+\)\s\+[0-9a-f]\+\s\+[0-9a-f]\+\s\+\([0-9a-f]\+\).*$/\1 \2/") # this gives us length and offset of the section
#echo $AVX_ARGS 1>&2
$BINARY $AVX_ARGS $2 $3 $4 $5 $6 $7
