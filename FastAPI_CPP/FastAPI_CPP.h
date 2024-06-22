// Tomas Costantino

#ifndef SERVERC___FASTAPI_CPP_H
#define SERVERC___FASTAPI_CPP_H


#include "http_lib.h"
#include <functional>
#include <vector>
#include <memory>
#include <iostream>
#include <atomic>
#include <csignal>

namespace fastapi_cpp {
    using Request = http::Request;
    using Response = http::Response;
    using Method = http::Method;

    class Route {
    public:
        virtual Response handle(const Request& request) const = 0;
        virtual ~Route() = default;
    };

    template<typename Func>
    class FunctionRoute : public Route {
        Method method;
        std::string path;
        Func handler;

    public:
        FunctionRoute(Method method, std::string p, Func h)
                : method(method), path(std::move(p)), handler(std::move(h)) {}

        Response handle(const Request& request) const override {
            if (request.method == method && request.uri == path) {
                return handler(request);
            }
            return http::HTTP_404_NOT_FOUND();
        }
    };

    class FastAPI {
    public:
        FastAPI();
        ~FastAPI();

        template<typename Func>
        void add_route(Method method, const std::string& path, Func handler) {
            routes.push_back(std::make_unique<FunctionRoute<Func>>(method, path, std::move(handler)));
        }
        void get(const std::string& path, std::function<Response(const Request&)> handler) {
            add_route(Method::GET, path, std::move(handler));
        }

        void post(const std::string& path, std::function<Response(const Request&)> handler) {
            add_route(Method::POST, path, std::move(handler));
        }

        void put(const std::string& path, std::function<Response(const Request&)> handler) {
            add_route(Method::PUT, path, std::move(handler));
        }

        void patch(const std::string& path, std::function<Response(const Request&)> handler) {
            add_route(Method::PATCH, path, std::move(handler));
        }

        void delete_(const std::string& path, std::function<Response(const Request&)> handler) {
            add_route(Method::DELETE, path, std::move(handler));
        }

        Response handle_request(const Request& req);
        void run(int port);
        void stop();

    private:
        std::vector<std::unique_ptr<Route>> routes;
        std::atomic<bool> running;
        int server_fd;

        static void signal_handler(int signal);
        static FastAPI* instance;
    };
}
#endif //SERVERC___FASTAPI_CPP_H
