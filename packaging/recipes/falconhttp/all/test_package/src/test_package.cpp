// Minimal smoke test: confirms the installed package's headers are
// reachable and the library links, by constructing core types and
// registering a route.
//
// This deliberately doesn't exercise parsing or dispatch behavior —
// that would need matching the library's actual API surface more
// closely. Consider expanding this to parse a raw request via
// HttpParser and dispatch it through Router once this is filled in
// for a real project.
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