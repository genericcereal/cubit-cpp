#!/bin/bash

# Build and run the simplified Qt Test suite

echo "Building Cubit test suite..."

# Clean any previous builds
rm -f cubit-tests

# Make sure we have the main project built first
cd ..
if [ ! -f "build/libcubit-objects.a" ]; then
    echo "Main project not built. Building it first..."
    ./build.sh
fi
cd tests

# Generate Makefile and build tests
qmake6 tests.pro
make

if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

echo "Running tests..."
echo "================"

# Run the tests
./cubit-tests

echo "================"
echo "Tests completed."