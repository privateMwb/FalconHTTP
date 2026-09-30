# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:23f294580c94a53c1625f5b98c7c22d7155fb030b877d0c2f937fbc8156b5aca

COPY . $SRC/FalconHTTP
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/FalconHTTP

# Unlike VectorPro, FalconHTTP depends on git submodules
# (libs/internal/*). Whatever checked out $SRC/FalconHTTP before the
# COPY above may or may not have fetched them recursively - this makes
# sure they're actually present regardless, rather than silently
# building against empty submodule directories.
RUN git submodule update --init --recursive
