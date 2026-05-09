FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libcurl4-openssl-dev \
    libssl-dev \
    ca-certificates \
    curl \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy files
COPY bot.cpp /app/bot.cpp
COPY config.env /app/config.env

# Fix: Compile with libraries in correct order and add -lcurl
RUN g++ -O3 -pthread -o userbot bot.cpp -lcurl -lssl -lcrypto

# Create non-root user
RUN groupadd -r userbot && useradd -r -g userbot userbot && \
    chown -R userbot:userbot /app

USER userbot

CMD ["./userbot"]
