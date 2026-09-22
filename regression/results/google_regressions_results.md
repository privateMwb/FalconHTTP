#FalconHTTPRegression Report

## cache

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| cache_hit | 90 ns | 90 ns | +0.0% |

## dispatch

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| dispatch_hit | 2.42 us | 2.42 us | +0.0% |
| dispatch_miss | 1.31 us | 1.31 us | +0.0% |

## match_stream

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| match_stream_hit | 1.66 us | 1.66 us | +0.0% |
| match_stream_miss | 1.04 us | 1.04 us | +0.0% |

## path

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| path_match | 1.14 us | 1.14 us | +0.0% |

## parse_body

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| parse_body_heavy | 14.75 us | 14.75 us | +0.0% |

## serialize_body

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| serialize_body_heavy | 13.03 us | 13.03 us | +0.0% |

## cache

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| cache_put | 425 ns | 425 ns | +0.0% |

## chain

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| chain_overhead | 744 ns | 744 ns | +0.0% |

## cors

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| cors_overhead | 4.18 us | 4.18 us | +0.0% |

## parse_header

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| parse_header_heavy | 21.52 us | 21.52 us | +0.0% |

## serialize_header

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| serialize_header_heavy | 5.63 us | 5.63 us | +0.0% |

## parse_request

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| parse_request_line | 2.13 us | 2.13 us | +0.0% |

## sse

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| sse_send | 4.73 us | 4.73 us | +0.0% |

## connection_move

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| connection_move_construction | 60.34 us | 60.34 us | +0.0% |

## server

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| server_construction | 224.64 us | 224.64 us | +0.0% |

## socket

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| socket_construction | 75.71 us | 75.71 us | +0.0% |

## parse_headers

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| parse_headers_5 | 3.87 us | 3.87 us | +0.0% |
| parse_headers_25 | 10.50 us | 10.50 us | +0.0% |
| parse_headers_100 | 41.51 us | 41.51 us | +0.0% |
| parse_headers_500 | 277.62 us | 277.62 us | +0.0% |

## chain_length

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| chain_length_1 | 1.49 us | 1.49 us | +0.0% |
| chain_length_5 | 1.58 us | 1.58 us | +0.0% |
| chain_length_20 | 2.04 us | 2.04 us | +0.0% |
| chain_length_50 | 40.16 us | 40.16 us | +0.0% |

## dispatch_routes

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| dispatch_routes_10 | 2.36 us | 2.36 us | +0.0% |
| dispatch_routes_100 | 16.25 us | 16.25 us | +0.0% |
| dispatch_routes_500 | 78.66 us | 78.66 us | +0.0% |
| dispatch_routes_2000 | 316.33 us | 316.33 us | +0.0% |

## method_from_string_case

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| method_from_string_case_best | 10 ns | 10 ns | +0.0% |
| method_from_string_case_worst | 23 ns | 23 ns | +0.0% |

## method_to

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| method_to_string | 5 ns | 5 ns | +0.0% |

## mime_lookup

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| mime_lookup_known | 12 ns | 12 ns | +0.0% |
| mime_lookup_unknown | 9 ns | 9 ns | +0.0% |

## url

| Benchmark | Current | v1.0.0 | Δ |
|---|---|---|---|
| url_decode | 1.34 us | 1.34 us | +0.0% |

## Summary

| Result | Count |
|---|---|
| Current faster | 0 (0%) |
| v1.0.0 faster | 0 (0%) |
| Tie | 36 (100%) |
