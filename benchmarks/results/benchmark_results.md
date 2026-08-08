# FalconHTTPBenchmark Results

## Cache Hit

| Test | Iteration | FalconHTTP|---|---|---|
| FileCache::get() Existing Entry | 10K | 916.08 us |
| FileCache::get() Existing Entry | 100K | 9.22 ms |
| FileCache::get() Existing Entry | 1M | 91.81 ms |

## Dispatch

| Test | Iteration | FalconHTTP|---|---|---|
| Router::dispatch() Matching Route | 10K | 24.81 ms |
| Router::dispatch() Matching Route | 100K | 249.70 ms |
| Router::dispatch() Matching Route | 1M | 5.70 s |
| Router::dispatch() No Matching Route | 10K | 29.22 ms |
| Router::dispatch() No Matching Route | 100K | 8.81 s |
| Router::dispatch() No Matching Route | 1M | 2.97 s |

## Path Match

| Test | Iteration | FalconHTTP|---|---|---|
| PathMatcher::match() Two Params | 10K | 19.26 ms |
| PathMatcher::match() Two Params | 100K | 169.44 ms |
| PathMatcher::match() Two Params | 1M | 1.68 s |

## Body

| Test | Iteration | FalconHTTP|---|---|---|
| Parse() 64 KiB Body | 10K | 239.82 ms |
| Parse() 64 KiB Body | 100K | 2.35 s |
| Parse() 64 KiB Body | 1M | 24.00 s |
| Serialize() 64 KiB Body | 10K | 219.14 ms |
| Serialize() 64 KiB Body | 100K | 2.27 s |
| Serialize() 64 KiB Body | 1M | 22.45 s |

## Cache Put

| Test | Iteration | FalconHTTP|---|---|---|
| FileCache::put() Fresh Key | 10K | 7.35 ms |
| FileCache::put() Fresh Key | 100K | 71.79 ms |
| FileCache::put() Fresh Key | 1M | 725.04 ms |

## Chain Overhead

| Test | Iteration | FalconHTTP|---|---|---|
| 3-middleware Chain + Handler | 10K | 17.00 ms |
| 3-middleware Chain + Handler | 100K | 168.98 ms |
| 3-middleware Chain + Handler | 1M | 1.73 s |

## Cors Overhead

| Test | Iteration | FalconHTTP|---|---|---|
| Cors::operator() Non-preflight | 10K | 61.10 ms |
| Cors::operator() Non-preflight | 100K | 612.87 ms |
| Cors::operator() Non-preflight | 1M | 6.26 s |

## Header

| Test | Iteration | FalconHTTP|---|---|---|
| Parse() 50 Headers | 10K | 421.71 ms |
| Parse() 50 Headers | 100K | 4.23 s |
| Parse() 50 Headers | 1M | 42.62 s |
| Serialize() 50 Headers | 10K | 115.16 ms |
| Serialize() 50 Headers | 100K | 1.11 s |
| Serialize() 50 Headers | 1M | 11.25 s |

## Request Line

| Test | Iteration | FalconHTTP|---|---|---|
| Parse() Minimal Request Line | 10K | 34.57 ms |
| Parse() Minimal Request Line | 100K | 347.81 ms |
| Parse() Minimal Request Line | 1M | 3.50 s |

## Connection Move

| Test | Iteration | FalconHTTP|---|---|---|
| Connection Create + Move Construction | 10K | 905.53 ms |
| Connection Create + Move Construction | 100K | 9.25 s |
| Connection Create + Move Construction | 1M | 92.23 s |

## Server Construction

| Test | Iteration | FalconHTTP|---|---|---|
| Server Construction, 4 Threads | 10K | 2.32 s |
| Server Construction, 4 Threads | 100K | 19.43 s |
| Server Construction, 4 Threads | 1M | 220.90 s |

## Socket Construction

| Test | Iteration | FalconHTTP|---|---|---|
| Socket::createTcp() + Close() | 10K | 940.52 ms |
| Socket::createTcp() + Close() | 100K | 9.45 s |
| Socket::createTcp() + Close() | 1M | 96.59 s |

## Header Count Growth

| Test | Iteration | FalconHTTP|---|---|---|
| Parse() 5 Headers | 10K | 69.00 ms |
| Parse() 5 Headers | 100K | 602.48 ms |
| Parse() 5 Headers | 1M | 6.00 s |
| Parse() 25 Headers | 10K | 247.60 ms |
| Parse() 25 Headers | 100K | 2.47 s |
| Parse() 25 Headers | 1M | 24.20 s |
| Parse() 100 Headers | 10K | 916.68 ms |
| Parse() 100 Headers | 100K | 9.17 s |
| Parse() 100 Headers | 1M | 1136.36 s |
| Parse() 500 Headers | 10K | 4.23 s |
| Parse() 500 Headers | 100K | 43.02 s |
| Parse() 500 Headers | 1M | 419.35 s |

## Middleware Chain Growth

| Test | Iteration | FalconHTTP|---|---|---|
| Chain Length 1 | 10K | 16.34 ms |
| Chain Length 1 | 100K | 163.69 ms |
| Chain Length 1 | 1M | 1.68 s |
| Chain Length 5 | 10K | 19.13 ms |
| Chain Length 5 | 100K | 186.27 ms |
| Chain Length 5 | 1M | 1.81 s |
| Chain Length 20 | 10K | 22.64 ms |
| Chain Length 20 | 100K | 233.09 ms |
| Chain Length 20 | 1M | 2.51 s |
| Chain Length 50 | 10K | 35.87 ms |
| Chain Length 50 | 100K | 408.69 ms |
| Chain Length 50 | 1M | 3.80 s |

## Route Table Growth

| Test | Iteration | FalconHTTP|---|---|---|
| Dispatch() Last Of 10 Routes | 10K | 28.77 ms |
| Dispatch() Last Of 10 Routes | 100K | 346.53 ms |
| Dispatch() Last Of 10 Routes | 1M | 3.03 s |
| Dispatch() Last Of 100 Routes | 10K | 215.97 ms |
| Dispatch() Last Of 100 Routes | 100K | 2.53 s |
| Dispatch() Last Of 100 Routes | 1M | 21.55 s |
| Dispatch() Last Of 500 Routes | 10K | 950.32 ms |
| Dispatch() Last Of 500 Routes | 100K | 9.75 s |
| Dispatch() Last Of 500 Routes | 1M | 98.65 s |
| Dispatch() Last Of 2000 Routes | 10K | 3.71 s |
| Dispatch() Last Of 2000 Routes | 100K | 37.90 s |
| Dispatch() Last Of 2000 Routes | 1M | 2100.40 s |

## Method Convert

| Test | Iteration | FalconHTTP|---|---|---|
| MethodFromString() "GET" (best Case) | 10K | 125.46 us |
| MethodFromString() "GET" (best Case) | 100K | 1.34 ms |
| MethodFromString() "GET" (best Case) | 1M | 12.55 ms |
| MethodFromString() "OPTIONS" (worst Case) | 10K | 291.31 us |
| MethodFromString() "OPTIONS" (worst Case) | 100K | 2.96 ms |
| MethodFromString() "OPTIONS" (worst Case) | 1M | 29.21 ms |
| MethodToString() Get | 10K | 55.77 us |
| MethodToString() Get | 100K | 620.92 us |
| MethodToString() Get | 1M | 6.29 ms |

## Mime Lookup

| Test | Iteration | FalconHTTP|---|---|---|
| MimeTypeFromExtension() Known Extension | 10K | 153.46 us |
| MimeTypeFromExtension() Known Extension | 100K | 1.52 ms |
| MimeTypeFromExtension() Known Extension | 1M | 15.32 ms |
| MimeTypeFromExtension() Unknown Extension | 10K | 110.54 us |
| MimeTypeFromExtension() Unknown Extension | 100K | 1.10 ms |
| MimeTypeFromExtension() Unknown Extension | 1M | 11.16 ms |

## Url Decode

| Test | Iteration | FalconHTTP|---|---|---|
| UrlDecoder::decode() Mixed Encoding | 10K | 15.97 ms |
| UrlDecoder::decode() Mixed Encoding | 100K | 158.89 ms |
| UrlDecoder::decode() Mixed Encoding | 1M | 1.60 s |
