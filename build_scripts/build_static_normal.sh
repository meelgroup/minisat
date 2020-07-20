#!/bin/bash

set -e

rm -rf lib* mini*
cmake -DSTATS_MODE=OFF -DPREDICT_MODE=OFF -DBIN_DRUP=ON -DSTATIC_BINARIES=ON ..
make -j6
