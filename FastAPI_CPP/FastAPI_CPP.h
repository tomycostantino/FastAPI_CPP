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
        virtual std::map<std::string, std::string> extract_params(const std::string& uri) const = 0;
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
            pattern += "(?:\\?.*)?$";

            path_regex = std::regex(pattern);
            std::cout << "Route created: " << method_to_string(method) << " " << path_pattern << std::endl;
            std::cout << "Regex pattern: " << pattern << std::endl;
        }

        bool matches(const Method& m, const std::string& uri) const override {
            if (method != m) return false;

            auto query_pos = uri.find('?');
            std::string path = (query_pos != std::string::npos) ? uri.substr(0, query_pos) : uri;

            return std::regex_match(path, path_regex);
        }


        std::map<std::string, std::string> extract_params(const std::string& uri) const override {
            std::map<std::string, std::string> params;
            std::smatch match;
            std::string uri_without_query = uri.substr(0, uri.find('?'));
            if (std::regex_match(uri_without_query, match, path_regex)) {
                for (size_t i = 0; i < param_names.size(); i++) {
                    params[param_names[i]] = match[i + 1].str();
                }
            }
            return params;
        }

        Response handle(const Request& request, const std::map<std::string, std::string>& params) const override {
            auto query_params = parse_query_string(request.uri);

            auto all_params = params;
            all_params.insert(query_params.begin(), query_params.end());

            return handler(request, all_params);
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
    private:
        std::map<std::string, std::string> parse_query_string(const std::string& uri) const {
            std::map<std::string, std::string> query_params;
            auto query_pos = uri.find('?');
            if (query_pos != std::string::npos) {
                std::string query = uri.substr(query_pos + 1);
                std::istringstream iss(query);
                std::string pair;
                while (std::getline(iss, pair, '&')) {
                    auto eq_pos = pair.find('=');
                    if (eq_pos != std::string::npos) {
                        std::string key = pair.substr(0, eq_pos);
                        std::string value = pair.substr(eq_pos + 1);
                        query_params[key] = value;
                    }
                }
            }
            return query_params;
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
