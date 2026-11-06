#!/bin/bash
set -e

echo "🔨 Building Pragma Once (Release Mode)..."
cmake --build build_release --parallel 4

echo "📦 Packaging deployment bundle..."
mkdir -p dist/PragmaOnce/bin
cp build_release/PragmaOnce dist/PragmaOnce/bin/

echo "✨ Pragma Once is packaged inside 'dist/PragmaOnce/'! Ready for desktop deployment."
