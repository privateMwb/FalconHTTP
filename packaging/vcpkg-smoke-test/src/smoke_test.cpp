// Minimal smoke test: confirms the vcpkg-installed package's headers
// are reachable and the library links, by constructing core types and
// registering a route.
//
// Same scope as the Conan test_package smoke test — proves the
// package is installable and linkable, not that its behavior is
// correct. Expand this to parse a raw request via HttpParser and
// dispatch it through Router once filled in for a real project.
#include <FalconHTTP/FalconHTTP.h>

#include <iostream>

int main() {
    rain::Routing::Router router;

    router.get("/health", [](const rain::HTTP::HttpRequest&, rain::HTTP::HttpResponse& response) {
        response.setStatus(rain::HTTP::HttpStatus::Ok);
    });

    std::cout << "FalconHTTP linked and constructed successfully.\n";
    return 0;
}
