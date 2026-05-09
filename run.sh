#!/bin/bash
# run.sh - Build and Run Script

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

clear

echo -e "${CYAN}"
echo "╔═══════════════════════════════════════════════════════════╗"
echo "║     🔥 VILLAIN USERBOT - C++ EDITION 🔥                  ║"
echo "║                                                           ║"
echo "║     ⚡ Full speed | 💾 Low memory | 🚀 Stable           ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo -e "${NC}"

if [ ! -f "config.env" ]; then
    echo -e "${RED}[!] config.env not found!${NC}"
    exit 1
fi

if [ ! -f "bot.cpp" ]; then
    echo -e "${RED}[!] bot.cpp not found!${NC}"
    exit 1
fi

if ! command -v g++ &> /dev/null; then
    echo -e "${YELLOW}[!] Installing g++...${NC}"
    apt-get update && apt-get install -y g++ libcurl4-openssl-dev libssl-dev
fi

if [ ! -f "./userbot" ] || [ "bot.cpp" -nt "./userbot" ]; then
    echo -e "${YELLOW}[!] Compiling...${NC}"
    g++ -O3 -pthread -o userbot bot.cpp -lcurl -lssl -lcrypto
    echo -e "${GREEN}[✓] Compilation complete!${NC}"
fi

echo -e "${GREEN}[✓] Starting Userbot...${NC}\n"
./userbot
