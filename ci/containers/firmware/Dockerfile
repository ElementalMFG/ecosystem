# SPDX-License-Identifier: Apache-2.0
# Copyright (c) 2025 SS-SP Project Contributors
#
# Reproducible firmware build container (S-02-001).
#
# Digest-pinned per the dependency-pin registry rules: identical bits for
# every developer and CI job. Tag v5.3.5 is recorded for humans; the digest
# is what Docker resolves.
#
# Bump procedure: update BOTH the comment tag and the digest in the same PR,
# with a changelog entry and a green `firmware-build` check (00_METHODOLOGY.md §5).
#
# espressif/idf:v5.3.5
FROM espressif/idf@sha256:114a9f8cde8bdc5a7e95809745b492663f7c5afe9c2821a4127ff133754fafd2

# Reproducibility hygiene: fixed locale/timezone; SOURCE_DATE_EPOCH is set by
# CI from the commit timestamp so embedded build metadata is deterministic.
ENV LC_ALL=C.UTF-8 \
    LANG=C.UTF-8 \
    TZ=UTC \
    IDF_CCACHE_ENABLE=1

WORKDIR /workspace

# The ESP-IDF entrypoint sources export.sh; build with e.g.:
#   docker run --rm -v "$PWD":/workspace -w /workspace/firmware \
#     $(docker build -q ci/containers/firmware) \
#     idf.py -B build/lite -DSS_BOARD=lite build
