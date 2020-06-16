#!/bin/bash

set -e

rm -rf lib* mini*
cmake -DPREDICT_MODE=ON -DSTATS_MODE=OFF ..
make -j6
