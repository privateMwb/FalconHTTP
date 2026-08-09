# FalconHTTPBenchmark Results

## Cache Hit

| Test | Iteration | FalconHTTP|---|---|---|
| FileCache::get() Existing Entry | 10K | 916.00 us |
| FileCache::get() Existing Entry | 100K | 9.21 ms |
| FileCache::get() Existing Entry | 1M | 92.46 ms |

## Dispatch

| Test | Iteration | FalconHTTP|---|---|---|
| Router::dispatch() Matching Route | 10K | 40.43 ms |
| Router::dispatch() Matching Route | 100K | 244.35 ms |
| Router::dispatch() Matching Route | 1M | 4.95 s |
| Router::dispatch() No Matching Route | 10K | 34.34 ms |
| Router::dispatch() No Matching Route | 100K | 338.35 ms |
| Router::dispatch() No Matching Route | 1M | 12.13 s |

## Path Match

| Test | Iteration | FalconHTTP|---|---|---|
| PathMatcher::match() Two Params | 10K | 21.37 ms |
| PathMatcher::match() Two Params | 100K | 194.35 ms |
| PathMatcher::match() Two Params | 1M | 2.00 s |

## Body

| Test | Iteration | FalconHTTP|---|---|---|
| Parse() 64 KiB Body | 10K | 273.11 ms |
| Parse() 64 KiB Body | 100K | 3.54 s |
| Parse() 64 KiB Body | 1M | 24.54 s |
| Serialize() 64 KiB Body | 10K | 233.57 ms |
| Serialize() 64 KiB Body | 100K | 2.27 s |
| Serialize() 64 KiB Body | 1M | 22.43 s |

## Cache Put

| Test | Iteration | FalconHTTP|---|---|---|
| FileCache::put() Fresh Key | 10K | 7.80 ms |
| FileCache::put() Fresh Key | 100K | 72.28 ms |
| FileCache::put() Fresh Key | 1M | 678.64 ms |

## Chain Overhead

| Test | Iteration | FalconHTTP|---|---|---|
| 3-middleware Chain + Handler | 10K | 16.84 ms |
| 3-middleware Chain + Handler | 100K | 167.94 ms |
| 3-middleware Chain + Handler | 1M | 1.90 s |

## Cors Overhead

| Test | Iteration | FalconHTTP|---|---|---|
| Cors::operator() Non-preflight | 10K | 64.42 ms |
| Cors::operator() Non-preflight | 100K | 612.99 ms |
| Cors::operator() Non-preflight | 1M | 6.30 s |

## Header

| Test | Iteration | FalconHTTP|---|---|---|
| Parse() 50 Headers | 10K | 426.15 ms |
| Parse() 50 Headers | 100K | 4.54 s |
| Parse() 50 Headers | 1M | 48.53 s |
| Serialize() 50 Headers | 10K | 121.20 ms |
| Serialize() 50 Headers | 100K | 1.22 s |
| Serialize() 50 Headers | 1M | 12.19 s |

## Request Line

| Test | Iteration | FalconHTTP|---|---|---|
| Parse() Minimal Request Line | 10K | 37.16 ms |
| Parse() Minimal Request Line | 100K | 378.42 ms |
| Parse() Minimal Request Line | 1M | 3.81 s |

## Connection Move

| Test | Iteration | FalconHTTP|---|---|---|
| Connection Create + Move Construction | 10K | 1.07 s |
| Connection Create + Move Construction | 100K | 10.64 s |
| Connection Create + Move Construction | 1M | 110.65 s |

## Server Construction

| Test | Iteration | FalconHTTP|---|---|---|
| Server Construction, 4 Threads | 10K | 3.27 s |
| Server Construction, 4 Threads | 100K | 24.39 s |
| Server Construction, 4 Threads | 1M | 246.20 s |

## Socket Construction

| Test | Iteration | FalconHTTP|---|---|---|
| Socket::createTcp() + Close() | 10K | 988.59 ms |
| Socket::createTcp() + Close() | 100K | 9.57 s |
| Socket::createTcp() + Close() | 1M | 682.75 s |

## Header Count Growth

| Test | Iteration | FalconHTTP|---|---|---|
| Parse() 5 Headers | 10K | 56.02 ms |
| Parse() 5 Headers | 100K | 541.16 ms |
| Parse() 5 Headers | 1M | 14.41 s |
| Parse() 25 Headers | 10K | 263.08 ms |
| Parse() 25 Headers | 100K | 2.22 s |
| Parse() 25 Headers | 1M | 22.66 s |
| Parse() 100 Headers | 10K | 832.51 ms |
| Parse() 100 Headers | 100K | 8.29 s |
| Parse() 100 Headers | 1M | 85.54 s |
| Parse() 500 Headers | 10K | 4.24 s |
| Parse() 500 Headers | 100K | 42.01 s |
| Parse() 500 Headers | 1M | 429.81 s |

## Middleware Chain Growth

| Test | Iteration | FalconHTTP|---|---|---|
| Chain Length 1 | 10K | 16.34 ms |
| Chain Length 1 | 100K | 164.16 ms |
| Chain Length 1 | 1M | 1.68 s |
| Chain Length 5 | 10K | 17.73 ms |
| Chain Length 5 | 100K | 176.26 ms |
| Chain Length 5 | 1M | 1.81 s |
| Chain Length 20 | 10K | 22.95 ms |
| Chain Length 20 | 100K | 225.10 ms |
| Chain Length 20 | 1M | 120.49 s |
| Chain Length 50 | 10K | 23.53 ms |
| Chain Length 50 | 100K | 178.25 ms |
| Chain Length 50 | 1M | 3.22 s |

## Route Table Growth

| Test | Iteration | FalconHTTP|---|---|---|
| Dispatch() Last Of 10 Routes | 10K | 29.23 ms |
| Dispatch() Last Of 10 Routes | 100K | 284.97 ms |
| Dispatch() Last Of 10 Routes | 1M | 11.72 s |
| Dispatch() Last Of 100 Routes | 10K | 238.67 ms |
| Dispatch() Last Of 100 Routes | 100K | 2.49 s |
| Dispatch() Last Of 100 Routes | 1M | 20.64 s |
| Dispatch() Last Of 500 Routes | 10K | 964.26 ms |
| Dispatch() Last Of 500 Routes | 100K | 10.18 s |
| Dispatch() Last Of 500 Routes | 1M | 96.56 s |
| Dispatch() Last Of 2000 Routes | 10K | 3.74 s |
| Dispatch() Last Of 2000 Routes | 100K | 37.46 s |
| Dispatch() Last Of 2000 Routes | 1M | 375.39 s |

## Method Convert

| Test | Iteration | FalconHTTP|---|---|---|
| MethodFromString() GET (best Case) | 10K | 118.08 us |
| MethodFromString() GET (best Case) | 100K | 1.17 ms |
| MethodFromString() GET (best Case) | 1M | 12.13 ms |
| MethodFromString() OPTIONS (worst Case) | 10K | 247.38 us |
| MethodFromString() OPTIONS (worst Case) | 100K | 2.46 ms |
| MethodFromString() OPTIONS (worst Case) | 1M | 24.96 ms |
| MethodToString() Get | 10K | 68.31 us |
| MethodToString() Get | 100K | 554.08 us |
| MethodToString() Get | 1M | 5.97 ms |

## Mime Lookup

| Test | Iteration | FalconHTTP|---|---|---|
| MimeTypeFromExtension() Known Extension | 10K | 174.77 us |
| MimeTypeFromExtension() Known Extension | 100K | 1.23 ms |
| MimeTypeFromExtension() Known Extension | 1M | 12.42 ms |
| MimeTypeFromExtension() Unknown Extension | 10K | 98.54 us |
| MimeTypeFromExtension() Unknown Extension | 100K | 985.00 us |
| MimeTypeFromExtension() Unknown Extension | 1M | 9.99 ms |

## Url Decode

| Test | Iteration | FalconHTTP|---|---|---|
| UrlDecoder::decode() Mixed Encoding | 10K | 14.24 ms |
| UrlDecoder::decode() Mixed Encoding | 100K | 144.75 ms |
| UrlDecoder::decode() Mixed Encoding | 1M | 1.45 s |
