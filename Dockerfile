FROM ubuntu:22.04 AS builder

# Install dependencies
RUN apt-get update && apt-get install -y \
    g++-11 \
    make \
    cmake \
    libcurl4-openssl-dev \
    libssl-dev \
    curl \
    && rm -rf /var/lib/apt/lists/* \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100

WORKDIR /app

# Copy source
COPY villain.bin /app/villain.cpp
COPY config.env /app/config.env

# Compile
RUN g++ -O3 -march=native -mtune=native -pthread -lcurl -lssl -lcrypto -o userbot villain.cpp

# Final stage
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
