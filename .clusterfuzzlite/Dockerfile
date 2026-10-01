# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:e67da2b0f6156b29b62734305a8c74db61aaf3158c7ab96386a07974fb33cdc8

COPY . $SRC/FalconHTTP
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/FalconHTTP

# Unlike VectorPro, FalconHTTP depends on git submodules
# (libs/internal/*). Whatever checked out $SRC/FalconHTTP before the
# COPY above may or may not have fetched them recursively - this makes
# sure they're actually present regardless, rather than silently
# building against empty submodule directories.
RUN git submodule update --init --recursive
