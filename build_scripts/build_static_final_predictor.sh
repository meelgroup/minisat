#!/bin/bash

set -e

rm -rf lib* mini*
cmake -DPREDICT_MODE=ON -DSTATS_MODE=OFF -DSTATIC_BINARIES=ON ..
make -j6
