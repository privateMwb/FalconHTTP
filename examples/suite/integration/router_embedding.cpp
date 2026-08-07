// Embedding Router behind a domain-specific API.
//
// Demonstrates:
// - Wrapping Router as a private implementation detail inside a class
// - Registering that class's routes in its constructor
// - Exposing a narrow, domain-specific surface instead of raw router.get()
// - Passing the wrapper's underlying Router straight into Server, since
//   Server only ever needs a Routing::Router&

#include <support/framework.h>

#include <iostream>
#include <unordered_map>

using namespace FalconHTTP;

namespace {

// A small in-memory "users" API. Router is a private member - callers
// interact with UserApi's own methods, not with route registration
// directly. This mirrors how an application might group related
// routes and their shared state behind one class.
class UserApi {
  public:
    UserApi() {
        users_["1"] = "Ada Lovelace";
        users_["2"] = "Alan Turing";

        router_.get("/users/:id",
                    [this](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
                        handleGetUser(request, response);
                    });

        router_.post("/users",
                     [this](const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
                         handleCreateUser(request, response);
                     });
    }

    // Exposes the embedded Router so it can be handed to a Server -
    // this is the only way UserApi's routes become reachable.
    Routing::Router& router() {
        return router_;
    }

  private:
    void handleGetUser(const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
        std::string id = request.pathParam("id");

        auto it = users_.find(id);
        if (it == users_.end()) {
            response.setStatus(HTTP::HttpStatus::NotFound);
            response.setBody("No such user");
            return;
        }

        response.setStatus(HTTP::HttpStatus::Ok);
        response.setBody(it->second);
    }

    void handleCreateUser(const HTTP::HttpRequest& request, HTTP::HttpResponse& response) {
        std::string nextId = std::to_string(users_.size() + 1);
        users_[nextId] = request.body();

        response.setStatus(HTTP::HttpStatus::Created);
        response.setBody("Created user " + nextId);
    }

    Routing::Router router_;
    std::unordered_map<std::string, std::string> users_;
};

} // namespace

static void run_examples() {

    // Construction registers every route up front - callers of UserApi
    // never see Router::get()/post() at all.
    setTitle("Constructing the Embedded API");

    UserApi api;

    std::cout << "UserApi constructed, routes registered internally\n\n";

    // Dispatch works exactly as it would against a router built by hand -
    // the wrapping is invisible from the request/response side.
    setTitle("Dispatching Through It");

    HTTP::HttpRequest request;
    request.setMethod(HTTP::HttpMethod::Get);
    request.setPath("/users/1");

    HTTP::HttpResponse response;
    (void)api.router().dispatch(request, response);

    std::cout << "status : " << static_cast<int>(response.status()) << "\n";
    std::cout << "body   : " << response.body() << "\n\n";

    // api.router() is what actually gets handed to a Server in a real
    // application: Core::Server server(api.router(), threadCount);
    setTitle("Handing It to a Server");

    Core::Server server(api.router(), /*threadCount=*/2);
    std::cout << "Server constructed from UserApi's embedded router\n";
}

REGISTER_EXAMPLE_SUITE();
