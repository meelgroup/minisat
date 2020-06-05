#!/bin/bash

set -e

rm -rf cm* CM* lib* cryptomini* Testing* tests* pycryptosat include tests
rm -f ../tests/cnf-files/*sqlite
cmake -DFINAL_PREDICTOR=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
make -j6
