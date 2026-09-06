#PROJECT_NAMERegression Report

## Cache Hit

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| FileCache::get() existing entry | 10K | 104.68 ns | 104.68 ns | +0.0% |
| FileCache::get() existing entry | 100K | 99.67 ns | 99.67 ns | +0.0% |
| FileCache::get() existing entry | 1M | 99.61 ns | 99.61 ns | +0.0% |

## Dispatch

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| Router::dispatch() matching route | 10K | 2656.22 ns | 2656.22 ns | +0.0% |
| Router::dispatch() matching route | 100K | 2649.32 ns | 2649.32 ns | +0.0% |
| Router::dispatch() matching route | 1M | 3563.44 ns | 3563.44 ns | +0.0% |
| Router::dispatch() no matching route | 10K | 1428.18 ns | 1428.18 ns | +0.0% |
| Router::dispatch() no matching route | 100K | 1681.63 ns | 1681.63 ns | +0.0% |
| Router::dispatch() no matching route | 1M | 1605.52 ns | 1605.52 ns | +0.0% |

## Match Stream

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| Router::matchStream() matching route | 10K | 3145.08 ns | 3145.08 ns | +0.0% |
| Router::matchStream() matching route | 100K | 3494.95 ns | 3494.95 ns | +0.0% |
| Router::matchStream() matching route | 1M | 2186.59 ns | 2186.59 ns | +0.0% |
| Router::matchStream() no matching route | 10K | 1168.02 ns | 1168.02 ns | +0.0% |
| Router::matchStream() no matching route | 100K | 1323.97 ns | 1323.97 ns | +0.0% |
| Router::matchStream() no matching route | 1M | 1195.53 ns | 1195.53 ns | +0.0% |

## Path Match

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| PathMatcher::match() two params | 10K | 782.4 ns | 782.4 ns | +0.0% |
| PathMatcher::match() two params | 100K | 1002.5 ns | 1002.5 ns | +0.0% |
| PathMatcher::match() two params | 1M | 1074.9 ns | 1074.9 ns | +0.0% |

## Body

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| parse() 64 KiB body | 10K | 11523.5 ns | 11523.5 ns | +0.0% |
| parse() 64 KiB body | 100K | 11308.6 ns | 11308.6 ns | +0.0% |
| parse() 64 KiB body | 1M | 32220.6 ns | 32220.6 ns | +0.0% |
| serialize() 64 KiB body | 10K | 30514.3 ns | 30514.3 ns | +0.0% |
| serialize() 64 KiB body | 100K | 26236 ns | 26236 ns | +0.0% |
| serialize() 64 KiB body | 1M | 25101.2 ns | 25101.2 ns | +0.0% |

## Cache Put

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| FileCache::put() fresh key | 10K | 1005.02 ns | 1005.02 ns | +0.0% |
| FileCache::put() fresh key | 100K | 845.95 ns | 845.95 ns | +0.0% |
| FileCache::put() fresh key | 1M | 831 ns | 831 ns | +0.0% |

## Chain Overhead

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| 3-middleware chain + handler | 10K | 1965.41 ns | 1965.41 ns | +0.0% |
| 3-middleware chain + handler | 100K | 1897.34 ns | 1897.34 ns | +0.0% |
| 3-middleware chain + handler | 1M | 1954.52 ns | 1954.52 ns | +0.0% |

## Cors Overhead

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| Cors::operator() non-preflight | 10K | 7891.14 ns | 7891.14 ns | +0.0% |
| Cors::operator() non-preflight | 100K | 7313.38 ns | 7313.38 ns | +0.0% |
| Cors::operator() non-preflight | 1M | 7237.37 ns | 7237.37 ns | +0.0% |

## Header

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| parse() 50 headers | 10K | 49213.8 ns | 49213.8 ns | +0.0% |
| parse() 50 headers | 100K | 52229.8 ns | 52229.8 ns | +0.0% |
| parse() 50 headers | 1M | 49652.8 ns | 49652.8 ns | +0.0% |
| serialize() 50 headers | 10K | 11885.4 ns | 11885.4 ns | +0.0% |
| serialize() 50 headers | 100K | 12311.5 ns | 12311.5 ns | +0.0% |
| serialize() 50 headers | 1M | 12386.9 ns | 12386.9 ns | +0.0% |

## Request Line

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| parse() minimal request line | 10K | 3861.95 ns | 3861.95 ns | +0.0% |
| parse() minimal request line | 100K | 3702.8 ns | 3702.8 ns | +0.0% |
| parse() minimal request line | 1M | 3777.68 ns | 3777.68 ns | +0.0% |

## Sse Send

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| SseConnection::send() framing + write | 10K | 12428 ns | 12428 ns | +0.0% |
| SseConnection::send() framing + write | 100K | 48303.9 ns | 48303.9 ns | +0.0% |
| SseConnection::send() framing + write | 1M | 19370.6 ns | 19370.6 ns | +0.0% |

## Connection Move

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| Connection create + move construction | 10K | 39465.7 ns | 39465.7 ns | +0.0% |
| Connection create + move construction | 100K | 42766.6 ns | 42766.6 ns | +0.0% |
| Connection create + move construction | 1M | 97256.8 ns | 97256.8 ns | +0.0% |

## Server Construction

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| Server construction, 4 threads | 10K | 345431 ns | 345431 ns | +0.0% |
| Server construction, 4 threads | 100K | 221493 ns | 221493 ns | +0.0% |
| Server construction, 4 threads | 1M | 302066 ns | 302066 ns | +0.0% |

## Socket Construction

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| Socket::createTcp() + close() | 10K | 101889 ns | 101889 ns | +0.0% |
| Socket::createTcp() + close() | 100K | 101973 ns | 101973 ns | +0.0% |
| Socket::createTcp() + close() | 1M | 113995 ns | 113995 ns | +0.0% |

## Header Count Growth

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| parse() 5 headers | 10K | 5914.3 ns | 5914.3 ns | +0.0% |
| parse() 5 headers | 100K | 5975.45 ns | 5975.45 ns | +0.0% |
| parse() 5 headers | 1M | 6195.26 ns | 6195.26 ns | +0.0% |
| parse() 25 headers | 10K | 24572.1 ns | 24572.1 ns | +0.0% |
| parse() 25 headers | 100K | 24464.2 ns | 24464.2 ns | +0.0% |
| parse() 25 headers | 1M | 25578.8 ns | 25578.8 ns | +0.0% |
| parse() 100 headers | 10K | 92583.5 ns | 92583.5 ns | +0.0% |
| parse() 100 headers | 100K | 99247.4 ns | 99247.4 ns | +0.0% |
| parse() 100 headers | 1M | 77548.8 ns | 77548.8 ns | +0.0% |
| parse() 500 headers | 10K | 234412 ns | 234412 ns | +0.0% |
| parse() 500 headers | 100K | 245804 ns | 245804 ns | +0.0% |
| parse() 500 headers | 1M | 462984 ns | 462984 ns | +0.0% |

## Middleware Chain Growth

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| chain length 1 | 10K | 1802.96 ns | 1802.96 ns | +0.0% |
| chain length 1 | 100K | 1830.74 ns | 1830.74 ns | +0.0% |
| chain length 1 | 1M | 1782.07 ns | 1782.07 ns | +0.0% |
| chain length 5 | 10K | 1901.47 ns | 1901.47 ns | +0.0% |
| chain length 5 | 100K | 1890.51 ns | 1890.51 ns | +0.0% |
| chain length 5 | 1M | 1896.17 ns | 1896.17 ns | +0.0% |
| chain length 20 | 10K | 2456.02 ns | 2456.02 ns | +0.0% |
| chain length 20 | 100K | 2447.83 ns | 2447.83 ns | +0.0% |
| chain length 20 | 1M | 2608.56 ns | 2608.56 ns | +0.0% |
| chain length 50 | 10K | 5305.32 ns | 5305.32 ns | +0.0% |
| chain length 50 | 100K | 4182.45 ns | 4182.45 ns | +0.0% |
| chain length 50 | 1M | 3545.35 ns | 3545.35 ns | +0.0% |

## Route Table Growth

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| dispatch() last of 10 routes | 10K | 2980.82 ns | 2980.82 ns | +0.0% |
| dispatch() last of 10 routes | 100K | 2964.19 ns | 2964.19 ns | +0.0% |
| dispatch() last of 10 routes | 1M | 2970.35 ns | 2970.35 ns | +0.0% |
| dispatch() last of 100 routes | 10K | 21595.4 ns | 21595.4 ns | +0.0% |
| dispatch() last of 100 routes | 100K | 21588.4 ns | 21588.4 ns | +0.0% |
| dispatch() last of 100 routes | 1M | 22273.6 ns | 22273.6 ns | +0.0% |
| dispatch() last of 500 routes | 10K | 104705 ns | 104705 ns | +0.0% |
| dispatch() last of 500 routes | 100K | 96226.6 ns | 96226.6 ns | +0.0% |
| dispatch() last of 500 routes | 1M | 100906 ns | 100906 ns | +0.0% |
| dispatch() last of 2000 routes | 10K | 413372 ns | 413372 ns | +0.0% |
| dispatch() last of 2000 routes | 100K | 415267 ns | 415267 ns | +0.0% |
| dispatch() last of 2000 routes | 1M | 430892 ns | 430892 ns | +0.0% |

## Method Convert

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| methodFromString() GET (best case) | 10K | 12.09 ns | 12.09 ns | +0.0% |
| methodFromString() GET (best case) | 100K | 12.02 ns | 12.02 ns | +0.0% |
| methodFromString() GET (best case) | 1M | 12.11 ns | 12.11 ns | +0.0% |
| methodFromString() OPTIONS (worst case) | 10K | 26.77 ns | 26.77 ns | +0.0% |
| methodFromString() OPTIONS (worst case) | 100K | 27.06 ns | 27.06 ns | +0.0% |
| methodFromString() OPTIONS (worst case) | 1M | 26.95 ns | 26.95 ns | +0.0% |
| methodToString() Get | 10K | 5.42 ns | 5.42 ns | +0.0% |
| methodToString() Get | 100K | 48.76 ns | 48.76 ns | +0.0% |
| methodToString() Get | 1M | 6.08 ns | 6.08 ns | +0.0% |

## Mime Lookup

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| mimeTypeFromExtension() known extension | 10K | 13.44 ns | 13.44 ns | +0.0% |
| mimeTypeFromExtension() known extension | 100K | 13.34 ns | 13.34 ns | +0.0% |
| mimeTypeFromExtension() known extension | 1M | 13.61 ns | 13.61 ns | +0.0% |
| mimeTypeFromExtension() unknown extension | 10K | 10.68 ns | 10.68 ns | +0.0% |
| mimeTypeFromExtension() unknown extension | 100K | 11.02 ns | 11.02 ns | +0.0% |
| mimeTypeFromExtension() unknown extension | 1M | 10.92 ns | 10.92 ns | +0.0% |

## Url Decode

| Test | Iteration | Current | Baseline | Δ |
|---|---|---|---|---|
| UrlDecoder::decode() mixed encoding | 10K | 1547.35 ns | 1547.35 ns | +0.0% |
| UrlDecoder::decode() mixed encoding | 100K | 1566.84 ns | 1566.84 ns | +0.0% |
| UrlDecoder::decode() mixed encoding | 1M | 1553.55 ns | 1553.55 ns | +0.0% |
