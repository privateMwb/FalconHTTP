# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:4d95d77396ce573bbfca794166cd2f1e28df5b3445a45132e52ef0e4ff7f566b

COPY . $SRC/FalconHTTP
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/FalconHTTP

# Unlike VectorPro, FalconHTTP depends on git submodules
# (libs/internal/*). Whatever checked out $SRC/FalconHTTP before the
# COPY above may or may not have fetched them recursively - this makes
# sure they're actually present regardless, rather than silently
# building against empty submodule directories.
RUN git submodule update --init --recursive
