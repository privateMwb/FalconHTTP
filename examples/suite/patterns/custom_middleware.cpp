// Writing a custom middleware.
//
// Demonstrates:
// - A MiddlewareFn is just anything matching its call signature - a
//   lambda works as well as a class with operator() (like Cors/Logger/Recovery)
// - Reading a request header before calling next()
// - Adding a response header after next() returns
// - Composing a custom middleware with the built-in Recovery middleware

#include <support/framework.h>

#include <iostream>
#include <stdexcept>

using namespace FalconHTTP;

namespace {
// Same recursive pattern Server::runChain() uses internally - stands
// in for a real Server here so the chain can be exercised directly.
void runChain(const Vector<Middleware::MiddlewareFn>& middleware, std::size_t index,
              HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
    if (index >= middleware.size()) {
        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody("handled");
        return;
    }

    Middleware::NextHandler next(
        [&middleware, index](HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
            runChain(middleware, index + 1, request, response);
        });

    middleware[index](request, response, next);
}
} // namespace

static void run_examples() {

    // A custom middleware just needs to match MiddlewareFn's signature:
    // void(HttpRequest&, HttpResponse&, const NextHandler&). This one
    // requires an API key header, rejecting the request before next() runs.
    setTitle("A Custom Middleware");

    Middleware::MiddlewareFn requireApiKey = [](HTTP::HttpRequest& request,
                                                HTTP::HttpResponse& response,
                                                const Middleware::NextHandler& next) {
        if (!request.hasHeader("X-Api-Key")) {
            response.setStatus(HTTP::HttpStatus::Unauthorized);
            response.setBody("Missing X-Api-Key header");
            return;
        }

        next(request, response);
        response.setHeader("X-Api-Key-Checked", "true");
    };

    Vector<Middleware::MiddlewareFn> chain;
    chain.push_back(requireApiKey);

    HTTP::HttpRequest unauthedRequest;
    HTTP::HttpResponse unauthedResponse;
    runChain(chain, 0, unauthedRequest, unauthedResponse);

    std::cout << "no key -> status : " << static_cast<int>(unauthedResponse.status()) << "\n";

    HTTP::HttpRequest authedRequest;
    authedRequest.setHeader("X-Api-Key", "secret");

    HTTP::HttpResponse authedResponse;
    runChain(chain, 0, authedRequest, authedResponse);

    std::cout << "with key -> status : " << static_cast<int>(authedResponse.status()) << "\n";
    std::cout << "checked header     : " << authedResponse.header("X-Api-Key-Checked") << "\n\n";

    // A custom middleware that throws is exactly what Recovery is for -
    // registering it first in the chain turns the exception into a clean
    // 500 instead of letting it propagate up and crash the handling thread.
    setTitle("Composing With Recovery");

    Middleware::MiddlewareFn explodes = [](HTTP::HttpRequest&, HTTP::HttpResponse&,
                                           const Middleware::NextHandler&) {
        throw std::runtime_error("something went wrong downstream");
    };

    Vector<Middleware::MiddlewareFn> protectedChain;
    protectedChain.push_back(Middleware::Recovery{});
    protectedChain.push_back(explodes);

    HTTP::HttpRequest request;
    HTTP::HttpResponse response;
    runChain(protectedChain, 0, request, response);

    std::cout << "status : " << static_cast<int>(response.status()) << "\n";
    std::cout << "body   : " << response.body() << "\n";
}

REGISTER_EXAMPLE_SUITE();
