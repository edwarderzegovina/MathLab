#!/bin/bash
set -euo pipefail
OUTPUT="proiect02"
rm -rf build-simple
mkdir -p build-simple
echo "=================== Building ===================="
g++ -std=c++17 -Wall -Wextra \
    -Isrc/core -Isrc/cli \
    src/core/IObject.cpp src/core/MathEntity.cpp src/core/AlgebraEntity.cpp src/core/DataEntity.cpp \
    src/core/Matrix.cpp src/core/Polynomial.cpp src/core/LinearSystem.cpp src/core/Dataset.cpp \
    src/core/SolutionReport.cpp src/core/DatasetSummary.cpp src/core/Workspace.cpp \
    src/core/MathLabException.cpp src/cli/ConsoleApp.cpp src/cli/ConsoleIO.cpp src/cli/main.cpp \
    -o "build-simple/$OUTPUT"
echo " Build succeeded: build-simple/$OUTPUT"
echo " Sample data lives in ./data — run from the project root."
