FROM debian:testing-slim

RUN \
    apt update \
    && apt install -y --no-install-recommends g++ cmake ninja-build libtag1-dev qtbase5-dev qtmultimedia5-dev
