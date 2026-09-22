# FalconHTTPBenchmark Results

## Cache Hit

| Test | Iteration | FalconHTTP |
|---|---|---|
| Get Existing Entry | 10K | 886.15 us |
| Get Existing Entry | 100K | 10.70 ms |
| Get Existing Entry | 1M | 90.76 ms |

## Dispatch

| Test | Iteration | FalconHTTP |
|---|---|---|
| Dispatch Match | 10K | 49.73 ms |
| Dispatch Match | 100K | 257.68 ms |
| Dispatch Match | 1M | 2.94 s |
| Dispatch No Match | 10K | 13.15 ms |
| Dispatch No Match | 100K | 196.36 ms |
| Dispatch No Match | 1M | 1.49 s |

## Match Stream

| Test | Iteration | FalconHTTP |
|---|---|---|
| MatchStream Match | 10K | 15.15 ms |
| MatchStream Match | 100K | 151.30 ms |
| MatchStream Match | 1M | 1.53 s |
| MatchStream No Match | 10K | 10.19 ms |
| MatchStream No Match | 100K | 101.31 ms |
| MatchStream No Match | 1M | 1.09 s |

## Path Match

| Test | Iteration | FalconHTTP |
|---|---|---|
| Match Two Params | 10K | 7.17 ms |
| Match Two Params | 100K | 134.18 ms |
| Match Two Params | 1M | 925.12 ms |

## Body

| Test | Iteration | FalconHTTP |
|---|---|---|
| Parse 64 KiB Body | 10K | 100.55 ms |
| Parse 64 KiB Body | 100K | 999.78 ms |
| Parse 64 KiB Body | 1M | 11.88 s |
| Serialize 64 KiB Body | 10K | 93.81 ms |
| Serialize 64 KiB Body | 100K | 934.06 ms |
| Serialize 64 KiB Body | 1M | 11.13 s |

## Cache Put

| Test | Iteration | FalconHTTP |
|---|---|---|
| Put Fresh Key | 10K | 4.49 ms |
| Put Fresh Key | 100K | 43.78 ms |
| Put Fresh Key | 1M | 413.32 ms |

## Chain Overhead

| Test | Iteration | FalconHTTP |
|---|---|---|
| Three-middleware Chain | 10K | 7.53 ms |
| Three-middleware Chain | 100K | 75.23 ms |
| Three-middleware Chain | 1M | 1.56 s |

## Cors Overhead

| Test | Iteration | FalconHTTP |
|---|---|---|
| Operator Non-preflight | 10K | 56.73 ms |
| Operator Non-preflight | 100K | 572.95 ms |
| Operator Non-preflight | 1M | 14.51 s |

## Header

| Test | Iteration | FalconHTTP |
|---|---|---|
| Parse 50 Headers | 10K | 378.20 ms |
| Parse 50 Headers | 100K | 3.89 s |
| Parse 50 Headers | 1M | 38.33 s |
| Serialize 50 Headers | 10K | 100.00 ms |
| Serialize 50 Headers | 100K | 996.36 ms |
| Serialize 50 Headers | 1M | 10.04 s |

## Request Line

| Test | Iteration | FalconHTTP |
|---|---|---|
| Parse Min Request Line | 10K | 30.54 ms |
| Parse Min Request Line | 100K | 307.04 ms |
| Parse Min Request Line | 1M | 3.07 s |

## Sse Send

| Test | Iteration | FalconHTTP |
|---|---|---|
| Send Framing + Write | 10K | 94.51 ms |
| Send Framing + Write | 100K | 12.33 s |
| Send Framing + Write | 1M | 56.39 s |

## Connection Move

| Test | Iteration | FalconHTTP |
|---|---|---|
| Conn Move Construct | 10K | 905.88 ms |
| Conn Move Construct | 100K | 8.92 s |
| Conn Move Construct | 1M | 90.03 s |

## Server Construction

| Test | Iteration | FalconHTTP |
|---|---|---|
| Server Construct 4threads | 10K | 1.89 s |
| Server Construct 4threads | 100K | 23.46 s |
| Server Construct 4threads | 1M | 321.94 s |

## Socket Construction

| Test | Iteration | FalconHTTP |
|---|---|---|
| CreateTcp + Close | 10K | 880.15 ms |
| CreateTcp + Close | 100K | 8.86 s |
| CreateTcp + Close | 1M | 88.87 s |

## Header Count Growth

| Test | Iteration | FalconHTTP |
|---|---|---|
| Parse 5 Headers | 10K | 50.35 ms |
| Parse 5 Headers | 100K | 482.04 ms |
| Parse 5 Headers | 1M | 4.82 s |
| Parse 25 Headers | 10K | 196.30 ms |
| Parse 25 Headers | 100K | 1.96 s |
| Parse 25 Headers | 1M | 19.63 s |
| Parse 100 Headers | 10K | 740.68 ms |
| Parse 100 Headers | 100K | 7.41 s |
| Parse 100 Headers | 1M | 73.72 s |
| Parse 500 Headers | 10K | 3.60 s |
| Parse 500 Headers | 100K | 36.00 s |
| Parse 500 Headers | 1M | 606.86 s |

## Middleware Chain Growth

| Test | Iteration | FalconHTTP |
|---|---|---|
| Chain Length 1 | 10K | 14.74 ms |
| Chain Length 1 | 100K | 147.09 ms |
| Chain Length 1 | 1M | 1.48 s |
| Chain Length 5 | 10K | 16.20 ms |
| Chain Length 5 | 100K | 157.03 ms |
| Chain Length 5 | 1M | 1.57 s |
| Chain Length 20 | 10K | 20.73 ms |
| Chain Length 20 | 100K | 206.53 ms |
| Chain Length 20 | 1M | 2.09 s |
| Chain Length 50 | 10K | 29.86 ms |
| Chain Length 50 | 100K | 293.99 ms |
| Chain Length 50 | 1M | 2.96 s |

## Route Table Growth

| Test | Iteration | FalconHTTP |
|---|---|---|
| Dispatch Last Of 10r | 10K | 24.08 ms |
| Dispatch Last Of 10r | 100K | 243.32 ms |
| Dispatch Last Of 10r | 1M | 2.41 s |
| Dispatch Last Of 100r | 10K | 163.10 ms |
| Dispatch Last Of 100r | 100K | 1.63 s |
| Dispatch Last Of 100r | 1M | 16.40 s |
| Dispatch Last Of 500r | 10K | 792.73 ms |
| Dispatch Last Of 500r | 100K | 7.95 s |
| Dispatch Last Of 500r | 1M | 80.09 s |
| Dispatch Last Of 2000r | 10K | 3.18 s |
| Dispatch Last Of 2000r | 100K | 32.17 s |
| Dispatch Last Of 2000r | 1M | 321.92 s |

## Method Convert

| Test | Iteration | FalconHTTP |
|---|---|---|
| MethodFromString GET | 10K | 100.92 us |
| MethodFromString GET | 100K | 1.00 ms |
| MethodFromString GET | 1M | 10.46 ms |
| MethodFromString OPTIONS | 10K | 228.92 us |
| MethodFromString OPTIONS | 100K | 2.46 ms |
| MethodFromString OPTIONS | 1M | 24.72 ms |
| MethodToString Get | 10K | 50.46 us |
| MethodToString Get | 100K | 510.85 us |
| MethodToString Get | 1M | 4.62 ms |

## Mime Lookup

| Test | Iteration | FalconHTTP |
|---|---|---|
| MimeType Known | 10K | 132.62 us |
| MimeType Known | 100K | 1.24 ms |
| MimeType Known | 1M | 12.52 ms |
| MimeType Unknown | 10K | 132.85 us |
| MimeType Unknown | 100K | 889.23 us |
| MimeType Unknown | 1M | 8.98 ms |

## Url Decode

| Test | Iteration | FalconHTTP |
|---|---|---|
| Decode Mixed Encoding | 10K | 13.02 ms |
| Decode Mixed Encoding | 100K | 129.57 ms |
| Decode Mixed Encoding | 1M | 1.30 s |
