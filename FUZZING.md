# Fuzzing

FalconHTTP is fuzzed via [ClusterFuzzLite](https://google.github.io/clusterfuzzlite/),
running on every pull request that touches a fuzzed file, plus a
longer scheduled batch run every night.

## What's covered

**`fuzz_http_parser.cpp`** targets `HttpParser::parse()` - the entry
point for every byte FalconHTTP ever reads off a socket, before any
of the library's own validation has run. Unlike a differential
fuzzer, there's no shadow-model HTTP parser to compare against, so
this instead:

- treats `parse()`'s own documented `std::runtime_error` (a missing
  CRLF, a request line without both a method and version, a header
  line without a colon) as an expected, non-finding outcome for
  malformed input
- on a successful parse, exercises every `HttpRequest` accessor -
  method/path/version/body, header/query/path-param lookups by name
  (both present and absent), and `json()` on the parsed body - so
  ASan/UBSan can catch anything reachable only through a specific
  field combination a hand-written unit test wouldn't think to
  construct

This codebase's own regression suite
(`bad_content_length`/`duplicate_header_fix`/`body_offset_fix`/
`oversized_body_rejected`/`oversized_header_rejected`, all under
`tests/suite/regression/`) is a track record of exactly the kind of
malformed-input bug this harness exists to catch before a human has
to find the next one by hand.

**`fuzz_path_matcher.cpp`** targets `PathMatcher::match()`, fuzzing
only the *path* argument against a small, fixed set of representative
patterns (literal segments, single `:param`, multiple `:param`). In
real usage a route's pattern is developer-written and registered once
at startup; the path is the side an attacker actually controls,
arriving fresh with every request - so this deliberately doesn't fuzz
both sides, which would mostly generate pattern/path pairs no real
route table would ever contain. It checks the one invariant
`PathMatcher.h` documents: a `true` result means every pattern segment
matched a path segment with nothing left over on either side, so the
two must have the same segment count. This is the open-ended
counterpart to the existing fixed path-traversal regression test.

Both are built and run under both AddressSanitizer and
UndefinedBehaviorSanitizer.

## What's deliberately NOT covered

- **`HttpSerializer`** - operates on a response FalconHTTP's own code
  already built, not on attacker-controlled bytes.
- **`Router::dispatch()`/`matchStream()` and `SseConnection`** -
  operate on an already-parsed `HttpRequest`/already-open connection,
  not raw bytes. `PathMatcher` (which they call into) is fuzzed
  directly instead, since that's where the actual attacker-controlled
  string lives.
- **Middleware** (`Cors`, `Logger`, `Recovery`) - operates on request/
  response objects that are, by the time middleware runs, already
  FalconHTTP's own well-formed data structures.
- **`JsonPro::Json::parse()` as its own target** - it's exercised
  indirectly via `fuzz_http_parser.cpp`'s call to
  `HttpRequest::json()`, but doesn't have a dedicated harness of its
  own yet in this repo. A natural follow-up if JsonPro doesn't already
  fuzz itself independently.

## Running locally

```bash
git clone --recursive https://github.com/google/oss-fuzz.git
cd oss-fuzz
python infra/helper.py build_fuzzers --sanitizer address FalconHTTP /path/to/FalconHTTP
python infra/helper.py run_fuzzer FalconHTTP fuzz_http_parser
python infra/helper.py run_fuzzer FalconHTTP fuzz_path_matcher
```

Or, without OSS-Fuzz's tooling, directly with clang (submodules must
already be checked out - `git submodule update --init --recursive`):

```bash
clang++ -std=c++20 -fsanitize=fuzzer,address \
  -Iinclude -Ilibs/internal/HashMapPro/include -Ilibs/internal/JsonParser/include \
  fuzz/fuzz_http_parser.cpp \
  src/FalconHTTP/HTTP/HttpParser.cpp \
  src/FalconHTTP/HTTP/HttpRequest.cpp \
  src/FalconHTTP/HTTP/HttpMethod.cpp \
  src/FalconHTTP/Utility/UrlDecoder.cpp \
  libs/internal/JsonParser/src/JsonPro/*.cpp \
  -o fuzz_http_parser

./fuzz_http_parser
```

```bash
clang++ -std=c++20 -fsanitize=fuzzer,address \
  -Iinclude -Ilibs/internal/HashMapPro/include \
  fuzz/fuzz_path_matcher.cpp \
  src/FalconHTTP/Routing/PathMatcher.cpp \
  -o fuzz_path_matcher

./fuzz_path_matcher
```

Add `-fsanitize=fuzzer,undefined` instead to run under UBSan.

## Reproducing a crash

ClusterFuzzLite uploads the failing input as a workflow artifact when
a run fails. Download it, then:

```bash
./fuzz_http_parser path/to/crash-<hash>
```

This replays that exact byte sequence through
`LLVMFuzzerTestOneInput()` once, deterministically - no sanitizer
flags needed beyond however the binary was already built.

## Adding a new harness

1. Add `fuzz/fuzz_<target>.cpp` with an `extern "C" int
   LLVMFuzzerTestOneInput(const uint8_t*, size_t)` entry point.
2. Add the matching compile + link block to
   `.clusterfuzzlite/build.sh` - list only the source files/submodules
   that harness actually needs (see the comment at the top of
   `build.sh`), not the whole library.
3. No workflow changes needed for `cflite_batch.yml` - it builds and
   runs every binary `build.sh` produces in `$OUT`. If the new harness
   targets different source files than the existing two, add them to
   `cflite_pr.yml`'s `paths:` filter so PRs touching them actually
   trigger a run.
