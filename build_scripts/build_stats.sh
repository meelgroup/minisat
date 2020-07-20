#!/bin/bash

set -e

rm -rf lib* mini*
cmake -DSTATS_MODE=ON -DPREDICT_MODE=OFF -DBIN_DRUP=ON -DSTATIC_BINARIES=OFF ..
make -j6
