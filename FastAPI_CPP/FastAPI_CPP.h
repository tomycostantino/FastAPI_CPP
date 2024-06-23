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
#include <regex>
#include <map>


namespace fastapi_cpp {
    using Request = http::Request;
    using Response = http::Response;
    using Method = http::Method;

    class Route {
    public:
        virtual Response handle(const Request& request, const std::map<std::string, std::string>& params) const = 0;
        virtual bool matches(const Method& method, const std::string& uri) const = 0;
        virtual const std::string& get_path_pattern() const = 0;
        virtual const std::regex& get_regex() const = 0;
        virtual const std::vector<std::string>& get_param_names() const = 0;
        virtual Method get_method() const = 0;
        virtual ~Route() = default;
    };

    template<typename Func>
    class FunctionRoute : public Route {
        Method method;
        std::string path_pattern;
        std::regex path_regex;
        std::vector<std::string> param_names;
        Func handler;

    public:
        FunctionRoute(Method m, std::string p, Func h)
                : method(m), path_pattern(std::move(p)), handler(std::move(h)) {
            std::string pattern = "^";
            std::regex param_regex(R"(\{([^}]+)\})");
            std::string::const_iterator search_start(path_pattern.cbegin());
            std::smatch match;

            while (std::regex_search(search_start, path_pattern.cend(), match, param_regex)) {
                pattern += std::string(search_start, match.prefix().second);
                pattern += "([^/]+)";
                param_names.push_back(match[1]);
                search_start = match.suffix().first;
            }

            pattern += std::string(search_start, path_pattern.cend());
            pattern += "$";

            path_regex = std::regex(pattern);
            std::cout << "Route created: " << method_to_string(method) << " " << path_pattern << std::endl;
            std::cout << "Regex pattern: " << pattern << std::endl;
        }

        bool matches(const Method& m, const std::string& uri) const override {
            bool method_matches = (method == m);
            bool uri_matches = std::regex_match(uri, path_regex);
            std::cout << "Matching route: " << method_to_string(method) << " " << path_pattern << std::endl;
            std::cout << "Against: " << method_to_string(m) << " " << uri << std::endl;
            std::cout << "Method match: " << (method_matches ? "true" : "false") << std::endl;
            std::cout << "URI match: " << (uri_matches ? "true" : "false") << std::endl;
            return method_matches && uri_matches;
        }

        Response handle(const Request& request, const std::map<std::string, std::string>& params) const override {
            return handler(request, params);
        }

        const std::string& get_path_pattern() const override {
            return path_pattern;
        }

        const std::regex& get_regex() const override {
            return path_regex;
        }

        const std::vector<std::string>& get_param_names() const override {
            return param_names;
        }

        Method get_method() const override {
            return method;
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

        void get(const std::string& path, std::function<Response(const Request&, const std::map<std::string, std::string>&)> handler) {
            add_route(Method::GET, path, std::move(handler));
        }

        void post(const std::string& path, std::function<Response(const Request&, const std::map<std::string, std::string>&)> handler) {
            add_route(Method::POST, path, std::move(handler));
        }

        void put(const std::string& path, std::function<Response(const Request&, const std::map<std::string, std::string>&)> handler) {
            add_route(Method::PUT, path, std::move(handler));
        }

        void patch(const std::string& path, std::function<Response(const Request&, const std::map<std::string, std::string>&)> handler) {
            add_route(Method::PATCH, path, std::move(handler));
        }

        void delete_(const std::string& path, std::function<Response(const Request&, const std::map<std::string, std::string>&)> handler) {
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
