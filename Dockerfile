FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libcurl4-openssl-dev \
    libssl-dev \
    ca-certificates \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY bot.cpp /app/bot.cpp
COPY config.env /app/config.env

# Compile
RUN g++ -O3 -pthread -lcurl -lssl -lcrypto -o userbot bot.cpp

# Create non-root user
RUN groupadd -r userbot && useradd -r -g userbot userbot && \
    chown -R userbot:userbot /app

USER userbot

CMD ["./userbot"]
