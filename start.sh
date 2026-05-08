#!/bin/bash
# start.sh - Build and run script

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
echo "║     ⚡ 30ms per message | 💾 50MB RAM                    ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Check if config exists
if [ ! -f "config.env" ]; then
    echo -e "${RED}[!] config.env not found!${NC}"
    exit 1
fi

# Check if binary exists
if [ ! -f "./userbot" ]; then
    echo -e "${YELLOW}[!] Compiling C++ userbot...${NC}"
    
    # Check for g++
    if ! command -v g++ &> /dev/null; then
        echo -e "${YELLOW}[!] g++ not found. Installing...${NC}"
        apt-get update && apt-get install -y g++ libcurl4-openssl-dev libssl-dev
    fi
    
    # Check if villain.cpp exists
    if [ ! -f "villain.cpp" ]; then
        echo -e "${RED}[!] villain.cpp not found!${NC}"
        exit 1
    fi
    
    # Compile (without -march=native for compatibility)
    g++ -O3 -pthread -lcurl -lssl -lcrypto -o userbot villain.cpp
    
    echo -e "${GREEN}[✓] Compilation complete!${NC}"
fi

# Run
echo -e "${GREEN}[✓] Starting Userbot...${NC}\n"
./userbot
