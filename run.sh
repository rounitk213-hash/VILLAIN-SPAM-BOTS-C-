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
echo "║     🔥 VILLAIN USERBOT - C++ ULTRA FAST 🔥               ║"
echo "║                                                           ║"
echo "║     ⚡ 100ms per message | 💾 50MB RAM                   ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Check config
if [ ! -f "config.env" ]; then
    echo -e "${RED}[!] config.env not found!${NC}"
    exit 1
fi

# Check if compiled
if [ ! -f "./userbot" ] || [ "bot.cpp" -nt "./userbot" ]; then
    echo -e "${YELLOW}[!] Compiling...${NC}"
    
    # Check for g++
    if ! command -v g++ &> /dev/null; then
        echo -e "${YELLOW}[!] Installing g++...${NC}"
        apt-get update && apt-get install -y g++ libcurl4-openssl-dev libssl-dev
    fi
    
    g++ -O3 -pthread -lcurl -lssl -lcrypto -o userbot bot.cpp
    echo -e "${GREEN}[✓] Compilation complete!${NC}"
fi

echo -e "${GREEN}[✓] Starting Userbot...${NC}\n"
./userbot
