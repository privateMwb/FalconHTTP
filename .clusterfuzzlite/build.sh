#!/bin/bash -eu
# ============================================================
# .clusterfuzzlite/build.sh
#
# Unlike VectorPro (header-only), FalconHTTP has separately-compiled
# .cpp sources, so each harness below compiles against a curated
# subset of them - not the whole library. Both current harnesses only
# exercise pure parsing/matching code, so Core/ (real sockets,
# ThreadPoolPro), Middleware/, Streaming/, and FileServer/ are
# deliberately left out: they're unreachable from either harness and
# would only add build time and a ThreadPoolPro submodule dependency
# neither one needs.
#
# Add more `${SRC}/FalconHTTP/fuzz/fuzz_*.cpp` harnesses here as
# they're added; each becomes its own $OUT binary. If a new harness
# needs a source file/submodule not already listed for it below, add
# it to that harness's own compile line - don't widen an existing
# harness's sources just to make a new one easier to bolt on.
# ============================================================

cd "${SRC}/FalconHTTP"

INCLUDES="-I${SRC}/FalconHTTP/include \
  -I${SRC}/FalconHTTP/libs/internal/HashMapPro/include \
  -I${SRC}/FalconHTTP/libs/internal/JsonParser/include"

# fuzz_http_parser: HttpParser + HttpRequest + HttpMethod + UrlDecoder
# (parsePath's percent-decoding), plus JsonParser's sources since the
# harness exercises HttpRequest::json() on the fuzzed body. find, not
# a flat *.cpp glob: JsonParser's sources live one level deeper, under
# src/JsonPro/, and a flat glob here would silently link zero of them.
JSON_SOURCES=$(find libs/internal/JsonParser/src -name '*.cpp')

$CXX $CXXFLAGS -std=c++20 $INCLUDES \
  fuzz/fuzz_http_parser.cpp \
  src/FalconHTTP/HTTP/HttpParser.cpp \
  src/FalconHTTP/HTTP/HttpRequest.cpp \
  src/FalconHTTP/HTTP/HttpMethod.cpp \
  src/FalconHTTP/Utility/UrlDecoder.cpp \
  $JSON_SOURCES \
  $LIB_FUZZING_ENGINE \
  -o "${OUT}/fuzz_http_parser"

# fuzz_path_matcher: PathMatcher only - it depends on nothing but
# HashMapPro (header-only, include-path only, no sources to compile).
$CXX $CXXFLAGS -std=c++20 $INCLUDES \
  fuzz/fuzz_path_matcher.cpp \
  src/FalconHTTP/Routing/PathMatcher.cpp \
  $LIB_FUZZING_ENGINE \
  -o "${OUT}/fuzz_path_matcher"
