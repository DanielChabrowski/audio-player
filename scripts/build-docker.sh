#!/usr/bin/env bash
set -euxo pipefail
DIST="${1:-debian:testing-slim}"
docker build --build-arg "BASE=${DIST}" -t audio-player-build .
docker run -v "$PWD:/src:ro" -v "$PWD/dist:/dist:rw" audio-player-build \
    bash -euxc ' \
        cp -r /src /code && \
        cd /code && \
        cmake . -GNinja && \
        ninja -v && \
        cp bin/foobar /dist/ \
    '
