#!/bin/bash

set -e

echo "============================================================"
echo "   🔥 VILLAIN USERBOT - C++ ULTRA FAST 🔥"
echo "============================================================"

if [ ! -f "config.env" ]; then
    echo "[!] config.env not found!"
    exit 1
fi

if [ ! -f "bot.cpp" ]; then
    echo "[!] bot.cpp not found!"
    exit 1
fi

if [ ! -f "./userbot" ] || [ "bot.cpp" -nt "./userbot" ]; then
    echo "[!] Compiling..."
    g++ -O3 -pthread -o userbot bot.cpp
    echo "[✓] Compilation complete!"
fi

echo "[✓] Starting Userbot..."
./userbot
