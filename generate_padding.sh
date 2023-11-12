#!/bin/sh
mkdir -p nitrofs
dd if=/dev/urandom of=nitrofs/pad.bin bs=1M count=16
