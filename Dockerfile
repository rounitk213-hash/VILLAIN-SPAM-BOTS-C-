FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    g++ \
    make \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY bot.cpp /app/bot.cpp
COPY config.env /app/config.env

RUN g++ -O3 -pthread -o userbot bot.cpp

RUN groupadd -r userbot && useradd -r -g userbot userbot && \
    chown -R userbot:userbot /app && \
    chmod +x /app/userbot

USER userbot

ENTRYPOINT ["/app/userbot"]
