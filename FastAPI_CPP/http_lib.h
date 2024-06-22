// Tomas Costantino

#ifndef HTTP_LIB_H
#define HTTP_LIB_H

#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>

namespace http {

    enum class Method {
        GET,
        HEAD,
        POST,
        PUT,
        PATCH,
        DELETE,
        CONNECT,
        OPTIONS,
        TRACE,
        UNKNOWN
    };

    enum class HttpStatus {
        OK = 200,
        CREATED = 201,
        ACCEPTED = 202,
        NO_CONTENT = 204,
        BAD_REQUEST = 400,
        UNAUTHORIZED = 401,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
        BAD_GATEWAY = 502,
        SERVICE_UNAVAILABLE = 503
    };

    struct Version {
        int major;
        int minor;
    };

    struct Request {
        Method method;
        std::string uri;
        Version version;
        std::map<std::string, std::string> headers;
        std::string body;
    };

    struct Response {
        Version version;
        HttpStatus status;
        std::map<std::string, std::string> headers;
        std::string body;

        std::string status_message() const {
            switch (status) {
                case HttpStatus::OK: return "OK";
                case HttpStatus::CREATED: return "Created";
                case HttpStatus::ACCEPTED: return "Accepted";
                case HttpStatus::NO_CONTENT: return "No Content";
                case HttpStatus::BAD_REQUEST: return "Bad Request";
                case HttpStatus::UNAUTHORIZED: return "Unauthorized";
                case HttpStatus::FORBIDDEN: return "Forbidden";
                case HttpStatus::NOT_FOUND: return "Not Found";
                case HttpStatus::METHOD_NOT_ALLOWED: return "Method Not Allowed";
                case HttpStatus::INTERNAL_SERVER_ERROR: return "Internal Server Error";
                case HttpStatus::NOT_IMPLEMENTED: return "Not Implemented";
                case HttpStatus::BAD_GATEWAY: return "Bad Gateway";
                case HttpStatus::SERVICE_UNAVAILABLE: return "Service Unavailable";
                default: return "Unknown Status";
            }
        }
    };

    inline std::string trim(const std::string& str) {
        auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
        auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
        return (start < end ? std::string(start, end) : std::string());
    }

    inline std::vector<std::string> split(const std::string& str, char delim) {
        std::vector<std::string> tokens;
        std::string token;
        std::istringstream tokenStream(str);
        while (std::getline(tokenStream, token, delim)) {
            tokens.push_back(token);
        }
        return tokens;
    }

    // Parsers
    inline Method string_to_method(const std::string& method) {
        if (method == "GET") return Method::GET;
        if (method == "HEAD") return Method::HEAD;
        if (method == "POST") return Method::POST;
        if (method == "PUT") return Method::PUT;
        if (method == "DELETE") return Method::DELETE;
        if (method == "CONNECT") return Method::CONNECT;
        if (method == "OPTIONS") return Method::OPTIONS;
        if (method == "TRACE") return Method::TRACE;
        if (method == "PATCH") return Method::PATCH;
        return Method::UNKNOWN;
    }

    inline std::string method_to_string(Method method) {
        switch (method) {
            case Method::GET: return "GET";
            case Method::HEAD: return "HEAD";
            case Method::POST: return "POST";
            case Method::PUT: return "PUT";
            case Method::DELETE: return "DELETE";
            case Method::CONNECT: return "CONNECT";
            case Method::OPTIONS: return "OPTIONS";
            case Method::TRACE: return "TRACE";
            case Method::PATCH: return "PATCH";
            default: return "UNKNOWN";
        }
    }

    inline Request parse_request(const std::string& raw_request) {
        Request request;
        std::istringstream stream(raw_request);
        std::string line;

        std::getline(stream, line);
        auto parts = split(line, ' ');
        if (parts.size() >= 3) {
            request.method = string_to_method(parts[0]);
            request.uri = parts[1];
            auto version_parts = split(parts[2].substr(5), '.');
            request.version = {std::stoi(version_parts[0]), std::stoi(version_parts[1])};
        }

        while (std::getline(stream, line) && line != "\r") {
            auto colon_pos = line.find(':');
            if (colon_pos != std::string::npos) {
                auto key = trim(line.substr(0, colon_pos));
                auto value = trim(line.substr(colon_pos + 1));
                request.headers[key] = value;
            }
        }

        std::string body((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
        request.body = body;

        return request;
    }

    inline std::string construct_response(const Response& response) {
        std::ostringstream stream;
        stream << "HTTP/" << response.version.major << "." << response.version.minor << " "
               << static_cast<int>(response.status) << " " << response.status_message() << "\r\n";

        for (const auto& header : response.headers) {
            stream << header.first << ": " << header.second << "\r\n";
        }

        stream << "\r\n" << response.body;
        return stream.str();
    }

    inline Response HTTP_200_OK(std::string body = "OK", std::map<std::string, std::string> headers = {{"Content-Type", "text/plain"}}) {
        return Response{{1, 1}, HttpStatus::OK, std::move(headers), std::move(body)};
    }

    inline Response HTTP_201_CREATED(std::string body = "Created", std::map<std::string, std::string> headers = {{"Content-Type", "text/plain"}}) {
        return Response{{1, 1}, HttpStatus::CREATED, std::move(headers), std::move(body)};
    }

    inline Response HTTP_400_BAD_REQUEST(std::string body = "Bad Request", std::map<std::string, std::string> headers = {{"Content-Type", "text/plain"}}) {
        return Response{{1, 1}, HttpStatus::BAD_REQUEST, std::move(headers), std::move(body)};
    }

    inline Response HTTP_404_NOT_FOUND(std::string body = "Not Found", std::map<std::string, std::string> headers = {{"Content-Type", "text/plain"}}) {
        return Response{{1, 1}, HttpStatus::NOT_FOUND, std::move(headers), std::move(body)};
    }

    inline Response HTTP_500_INTERNAL_SERVER_ERROR(std::string body = "Internal Server Error", std::map<std::string, std::string> headers = {{"Content-Type", "text/plain"}}) {
        return Response{{1, 1}, HttpStatus::INTERNAL_SERVER_ERROR, std::move(headers), std::move(body)};
    }

    inline Response custom_response(HttpStatus status, std::string body = "", std::map<std::string, std::string> headers = {{"Content-Type", "text/plain"}}) {
        return Response{{1, 1}, status, std::move(headers), std::move(body)};
    }
}

#endif