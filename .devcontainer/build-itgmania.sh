#!/bin/bash
set -e

# Define build type (Debug or Release)
BUILD_TYPE=${1:-Debug}

# Define build directory
BUILD_DIR="Build"

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}Configuring ITGmania build (${BUILD_TYPE})...${NC}"
cmake -S . -B $BUILD_DIR -DCMAKE_BUILD_TYPE=$BUILD_TYPE

echo -e "${GREEN}Building ITGmania...${NC}"
cmake --build $BUILD_DIR -j $(nproc)

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build completed successfully!${NC}"
    echo -e "${YELLOW}You can run ITGmania with: ${NC}$BUILD_DIR/ITGmania"
else
    echo -e "${RED}Build failed.${NC}"
    exit 1
fi 