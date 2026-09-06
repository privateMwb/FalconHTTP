# ============================================================
# .clusterfuzzlite/Dockerfile
# Base image for building the fuzz harnesses under ClusterFuzzLite.
# ============================================================

FROM gcr.io/oss-fuzz-base/base-builder@sha256:8b4a73d83374b298a0a771eeec9a1d44ceff3416a678b7d7946303389d022aca

COPY . $SRC/FalconHTTP
COPY .clusterfuzzlite/build.sh $SRC/

WORKDIR $SRC/FalconHTTP

# Unlike VectorPro, FalconHTTP depends on git submodules
# (libs/internal/*). Whatever checked out $SRC/FalconHTTP before the
# COPY above may or may not have fetched them recursively - this makes
# sure they're actually present regardless, rather than silently
# building against empty submodule directories.
RUN git submodule update --init --recursive
