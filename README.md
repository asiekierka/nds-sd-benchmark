# dldi-driver-benchmark

Simple filesystem I/O benchmark for NDS homebrew.

The benchmark expects an 8 MB file "benchmark_pad.bin" in the root of the CF/SD card.
This file will be created automatically if it doesn't exist, but you might want to create it manually
for read-only storage mediums.

## Building

    $ make -f Makefile.blocks # or Makefile.dkp

## License

MIT
