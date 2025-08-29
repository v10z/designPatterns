#!/bin/bash
# Build script for Linux/macOS - C++ Design Patterns

echo "====================================="
echo "Building C++ Design Patterns"
echo "====================================="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for C++ compiler
if command -v g++ &> /dev/null; then
    CXX=g++
elif command -v clang++ &> /dev/null; then
    CXX=clang++
else
    echo -e "${RED}Error: No C++ compiler found (g++ or clang++)${NC}"
    echo "Please install a C++ compiler:"
    echo "  Ubuntu/Debian: sudo apt-get install g++"
    echo "  macOS: xcode-select --install"
    echo "  Fedora: sudo dnf install gcc-c++"
    exit 1
fi

echo -e "Using compiler: ${GREEN}$CXX${NC}"

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
            
            # Try C++17 first, fall back to C++14
            if $CXX -std=c++17 -Wall -O2 "$cpp_file" -o "build/$pattern_name" 2>/dev/null; then
                echo -e "${GREEN}[OK]${NC} $pattern_name built successfully"
                ((success++))
            elif $CXX -std=c++14 -Wall -O2 "$cpp_file" -o "build/$pattern_name" 2>/dev/null; then
                echo -e "${GREEN}[OK]${NC} $pattern_name built successfully (C++14)"
                ((success++))
            elif $CXX -std=c++11 -Wall -O2 "$cpp_file" -o "build/$pattern_name" 2>/dev/null; then
                echo -e "${GREEN}[OK]${NC} $pattern_name built successfully (C++11)"
                ((success++))
            else
                echo -e "${RED}[FAILED]${NC} $pattern_name build failed"
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
    echo -e "${YELLOW}Some builds failed. This may be due to:${NC}"
    echo "- C++17/20 features not supported by your compiler"
    echo "- Missing dependencies for specific patterns"
    echo "Try updating your compiler:"
    echo "  g++ --version (should be 7.0+ for C++17, 10.0+ for C++20)"
fi

echo
if [ $success -gt 0 ]; then
    echo -e "${GREEN}Built patterns are in: $(pwd)/build/${NC}"
fi

# Make script executable
chmod +x build_all.sh 2>/dev/null