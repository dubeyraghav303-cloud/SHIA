#!/usr/bin/env bash

# deploy_engine.sh
# Automates the setup and compilation of the SHIA engine on a fresh Ubuntu server.

set -e

echo "==========================================="
echo "   SHIA Production Deployment Script       "
echo "==========================================="

echo "1. Updating system and installing dependencies..."
sudo apt-get update -y
sudo apt-get install -y build-essential cmake libcurl4-openssl-dev python3-requests

echo "2. Setting up Engine..."
# In a real scenario, this would git fetch / pull latest
# Here we assume the script is executed from the repo root

if [ ! -d "build" ]; then
    mkdir build
fi

cd build
echo "3. Cleaning previous builds..."
rm -rf *

echo "4. Running CMake..."
cmake ..

echo "5. Compiling C++ binary..."
make

echo "6. Injecting Production Configuration..."
cd ..
if [ -f "core/config/shia_config.production.json" ]; then
    cp core/config/shia_config.production.json core/config/shia_config.json
    echo " -> Production configuration injected successfully."
else
    echo " -> ⚠️ Warning: core/config/shia_config.production.json not found."
fi

echo "==========================================="
echo "✅ Deployment Successful!"
echo "Run the engine with: ./app/SHIA.app/Contents/MacOS/shia_engine"
echo "(Or preferably set it up as a systemd service.)"
