#!/bin/bash

set -e

rm -rf lib* mini*
cmake -DSTATS_MODE=OFF -DPREDICT_MODE=OFF -DBIN_DRUP=ON ..
make -j6
