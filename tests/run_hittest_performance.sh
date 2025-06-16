#!/bin/bash

# Build and run the HitTest performance test

echo "Building HitTest performance test..."
qmake6 -o Makefile.hittest test_hittest_performance.pro || exit 1
make -f Makefile.hittest -j8 || exit 1

echo "Running HitTest performance test..."
./test_hittest_performance.app/Contents/MacOS/test_hittest_performance