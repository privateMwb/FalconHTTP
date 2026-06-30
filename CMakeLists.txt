cmake_minimum_required(VERSION 3.20)
project(FalconHTTP LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library
add_library(FalconHTTPLib STATIC

    # Core
    src/Core/Socket.cpp
    src/Core/Listener.cpp
    src/Core/Connection.cpp
    src/Core/Server.cpp

    # HTTP
    src/HTTP/HttpMethod.cpp
    src/HTTP/HttpRequest.cpp
    src/HTTP/HttpParser.cpp
    src/HTTP/HttpStatus.cpp
    src/HTTP/HttpResponse.cpp
    src/HTTP/HttpSerializer.cpp
    src/HTTP/MimeTypes.cpp

    # Routing
    src/Routing/Route.cpp
    src/Routing/PathMatcher.cpp
    src/Routing/Router.cpp

    # Middleware
    src/Middleware/Logger.cpp
    src/Middleware/Cors.cpp
    src/Middleware/Recovery.cpp

    # File Server
    src/FileServer/StaticFileServer.cpp
    src/FileServer/FileCache.cpp

    # Utility
    src/Utility/UrlDecoder.cpp

    # External libs
    libs/JsonParser/Json.cpp
    libs/JsonParser/Parser.cpp
    libs/PulseThreadPool/ThreadPool.cpp
)

# Include directories (LONG INCLUDE STYLE ENABLED)
target_include_directories(FalconHTTPLib PUBLIC
    # Public API root
    include

    # Enables: #include "Core/Connection.h"
    src

    # Internal libs (still usable inside project)
    libs/HashMap
    libs/VectorPro
    libs/JsonParser
    libs/FunctionPro
    libs/LRUCache
    libs/PulseThreadPool
)

# Tests
add_executable(tests
    tests/test_all.cpp
    tests/test_http_parser.cpp
    tests/test_http_serializer.cpp
    tests/test_router.cpp
    tests/test_path_matcher.cpp
    tests/test_url_decoder.cpp
    tests/test_static_file_server.cpp
    tests/test_middleware.cpp
)

target_link_libraries(tests PRIVATE FalconHTTPLib)

# Examples
add_executable(basic_server
    examples/basic_server.cpp
)

target_link_libraries(basic_server PRIVATE FalconHTTPLib)