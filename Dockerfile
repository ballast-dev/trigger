FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive
ENV DEBCONF_NONINTERACTIVE_SEEN=true

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    devscripts \
    debhelper \
    dh-make \
    fakeroot \
    lintian \
    pbuilder \
    && rm -rf /var/lib/apt/lists/*

RUN wget -q https://github.com/casey/just/releases/download/1.41.0/just-1.41.0-x86_64-unknown-linux-musl.tar.gz \
    && tar -xzf just-1.41.0-x86_64-unknown-linux-musl.tar.gz -C /usr/local/bin just \
    && rm just-1.41.0-x86_64-unknown-linux-musl.tar.gz

WORKDIR /app

CMD ["/bin/bash"]
