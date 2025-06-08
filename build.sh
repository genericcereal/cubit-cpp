#!/bin/bash

# Build script for Qt Quick version

echo "Building Qt Quick version of Cubit..."

# Clean previous build
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Create build directory
mkdir -p build
cd build

# Run qmake
echo "Running qmake..."
qmake6 ../cubit-quick.pro

# Build
echo "Building..."
make -j$(nproc)

if [ $? -eq 0 ]; then
    echo "Build successful!"
    echo "Run ./build/cubit-quick to start the application"
else
    echo "Build failed!"
    exit 1
fi