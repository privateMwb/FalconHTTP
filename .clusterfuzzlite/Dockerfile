# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:df8728a13b864d0db206ba6daee06fcdb812cf82111c41d54d8db4d6e585437e

COPY . $SRC/FalconHTTP
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/FalconHTTP

# Unlike VectorPro, FalconHTTP depends on git submodules
# (libs/internal/*). Whatever checked out $SRC/FalconHTTP before the
# COPY above may or may not have fetched them recursively - this makes
# sure they're actually present regardless, rather than silently
# building against empty submodule directories.
RUN git submodule update --init --recursive
