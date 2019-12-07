ARG BASE=debian:testing-slim
FROM ${BASE}

RUN : \
    && apt-get update \
    && apt-get install -y --no-install-recommends \
        git \
        cmake \
        g++ \
        libtag1-dev \
        ninja-build \
        qtbase5-dev \
        qtmultimedia5-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
