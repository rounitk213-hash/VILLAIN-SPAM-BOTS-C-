FROM ubuntu:22.04 AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    libcurl4-openssl-dev \
    libssl-dev \
    curl \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY villain.cpp /app/villain.cpp
COPY config.env /app/config.env

# Compile with optimizations (without -march=native for compatibility)
RUN g++ -O3 -pthread -lcurl -lssl -lcrypto -o userbot villain.cpp

FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    libcurl4 \
    libssl3 \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd -r userbot && useradd -r -g userbot userbot

COPY --from=builder /app/userbot /usr/local/bin/
COPY --from=builder /app/config.env /app/

RUN chmod +x /usr/local/bin/userbot

WORKDIR /app
USER userbot

ENTRYPOINT ["/usr/local/bin/userbot"]
