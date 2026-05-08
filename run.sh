#!/bin/bash
# run.sh - Build and Run Script for C++ Userbot

set -e

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m'

clear

echo -e "${CYAN}"
echo "╔════════════════════════════════════════════════════════════════╗"
echo "║                                                                ║"
echo "║     ██╗   ██╗██╗██╗     ██╗      █████╗ ██╗███╗   ██╗         ║"
echo "║     ██║   ██║██║██║     ██║     ██╔══██╗██║████╗  ██║         ║"
echo "║     ██║   ██║██║██║     ██║     ███████║██║██╔██╗ ██║         ║"
echo "║     ╚██╗ ██╔╝██║██║     ██║     ██╔══██║██║██║╚██╗██║         ║"
echo "║      ╚████╔╝ ██║███████╗███████╗██║  ██║██║██║ ╚████║         ║"
echo "║       ╚═══╝  ╚═╝╚══════╝╚══════╝╚═╝  ╚═╝╚═╝╚═╝  ╚═══╝         ║"
echo "║                                                                ║"
echo "║         🔥 C++ ULTRA FAST USERBOT - NO FLOOD LIMITS 🔥        ║"
echo "║                                                                ║"
echo "║         ⚡ 30ms per message | 100+ concurrent tasks           ║"
echo "║         💾 50MB RAM | 📡 Zero flood limits                    ║"
echo "║                                                                ║"
echo "╚════════════════════════════════════════════════════════════════╝"
echo -e "${NC}"

# Check requirements
echo -e "${YELLOW}[!] Checking requirements...${NC}"

# Check for g++
if ! command -v g++ &> /dev/null; then
    echo -e "${RED}[!] g++ not found! Installing...${NC}"
    apt-get update && apt-get install -y g++ make libcurl4-openssl-dev libssl-dev
fi

# Check for curl
if ! command -v curl &> /dev/null; then
    echo -e "${RED}[!] curl not found! Installing...${NC}"
    apt-get install -y curl
fi

# Check config
if [ ! -f "config.env" ]; then
    echo -e "${RED}[!] config.env not found!${NC}"
    echo -e "${YELLOW}Creating template config.env...${NC}"
    cat > config.env << 'EOF'
# Telegram API Configuration
API_ID=your_api_id
API_HASH=your_api_hash
MAIN_OWNER=your_telegram_id
AUTO_JOIN_LINK=https://t.me/TheVillainActive

# Performance Settings
SEND_DELAY_MS=30
MAX_CONCURRENT=100
BATCH_SIZE=50
MESSAGE_QUEUE_SIZE=10000

# Session (format: SESSION_INDEX=session_string||owner_id)
SESSION_0=your_session_string||your_owner_id
EOF
    echo -e "${RED}Please edit config.env with your details and run again!${NC}"
    exit 1
fi

# Check if bot.cpp exists
if [ ! -f "bot.cpp" ]; then
    echo -e "${RED}[!] bot.cpp not found!${NC}"
    exit 1
fi

# Compile if binary doesn't exist or source is newer
if [ ! -f "./userbot" ] || [ "bot.cpp" -nt "./userbot" ]; then
    echo -e "${YELLOW}[!] Compiling C++ userbot...${NC}"
    
    # Compile with optimizations
    g++ -O3 -pthread -lcurl -lssl -lcrypto -o userbot bot.cpp
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}[✓] Compilation successful!${NC}"
    else
        echo -e "${RED}[✗] Compilation failed!${NC}"
        exit 1
    fi
fi

# Run with high priority
echo -e "${GREEN}[✓] Starting Userbot...${NC}\n"
echo -e "${CYAN}Press Ctrl+C to stop${NC}\n"

# Run with nice priority for better performance
nice -n -10 ./userbot

echo -e "\n${YELLOW}Userbot stopped${NC}"
