# Multi-stage build for smallest image
FROM ubuntu:22.04 AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libcurl4-openssl-dev \
    libssl-dev \
    curl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source
COPY bot.cpp /app/bot.cpp
COPY config.env /app/config.env

# Compile with optimizations
RUN g++ -O3 -pthread -lcurl -lssl -lcrypto -o userbot bot.cpp

# Final stage
FROM ubuntu:22.04

# Install runtime dependencies only
RUN apt-get update && apt-get install -y \
    libcurl4 \
    libssl3 \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd -r userbot && useradd -r -g userbot userbot

# Copy binary and config
COPY --from=builder /app/userbot /usr/local/bin/
COPY --from=builder /app/config.env /app/

# Set permissions
RUN chmod +x /usr/local/bin/userbot && \
    chown -R userbot:userbot /app

WORKDIR /app
USER userbot

# Run
ENTRYPOINT ["/usr/local/bin/userbot"]
