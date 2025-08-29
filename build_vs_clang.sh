#!/bin/bash
# Build script for VS2022 Clang++ - C++ Design Patterns

echo "====================================="
echo "Building with VS2022 Clang++ 19.1.5"
echo "====================================="

# VS Clang++ path
CXX="/c/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/Llvm/x64/bin/clang++.exe"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "Using compiler: ${GREEN}VS2022 Clang++ 19.1.5${NC}"

# Create build directory
mkdir -p build

# Counters
success=0
failed=0

# Build each pattern directory
for dir in [0-9][0-9]-*/; do
    if [ -d "$dir" ]; then
        pattern_name=$(basename "$dir")
        # Extract pattern name without number prefix (e.g., "singleton" from "01-singleton")
        pattern_only="${pattern_name#*-}"
        # Replace hyphens with underscores
        filename="${pattern_only//-/_}"
        cpp_file="$dir${filename}.cpp"
        
        if [ -f "$cpp_file" ]; then
            echo -e "\nBuilding ${YELLOW}$pattern_name${NC}..."
            
            # Try C++20 first (for newer patterns), then C++17, C++14, C++11
            if "$CXX" -std=c++20 -Wall -O2 "$cpp_file" -o "build/$pattern_name.exe" 2>/dev/null; then
                echo -e "${GREEN}[OK]${NC} $pattern_name built successfully (C++20)"
                ((success++))
            elif "$CXX" -std=c++17 -Wall -O2 "$cpp_file" -o "build/$pattern_name.exe" 2>/dev/null; then
                echo -e "${GREEN}[OK]${NC} $pattern_name built successfully (C++17)"
                ((success++))
            elif "$CXX" -std=c++14 -Wall -O2 "$cpp_file" -o "build/$pattern_name.exe" 2>/dev/null; then
                echo -e "${GREEN}[OK]${NC} $pattern_name built successfully (C++14)"
                ((success++))
            elif "$CXX" -std=c++11 -Wall -O2 "$cpp_file" -o "build/$pattern_name.exe" 2>/dev/null; then
                echo -e "${GREEN}[OK]${NC} $pattern_name built successfully (C++11)"
                ((success++))
            else
                echo -e "${RED}[FAILED]${NC} $pattern_name build failed"
                # Show error for debugging
                echo "Error details:"
                "$CXX" -std=c++20 -Wall -O2 "$cpp_file" -o "build/$pattern_name.exe" 2>&1 | head -5
                ((failed++))
            fi
        fi
    fi
done

echo
echo "====================================="
echo "Build Summary:"
echo -e "Successful: ${GREEN}$success${NC}"
echo -e "Failed: ${RED}$failed${NC}"
echo "====================================="

if [ $failed -gt 0 ]; then
    echo
    echo -e "${YELLOW}Some builds failed due to code errors.${NC}"
fi

echo
if [ $success -gt 0 ]; then
    echo -e "${GREEN}Built patterns are in: $(pwd)/build/${NC}"
fi