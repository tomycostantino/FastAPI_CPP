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
        int status_code;
        std::string status_message;
        std::map<std::string, std::string> headers;
        std::string body;
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
               << response.status_code << " " << response.status_message << "\r\n";

        for (const auto& header : response.headers) {
            stream << header.first << ": " << header.second << "\r\n";
        }

        stream << "\r\n" << response.body;

        return stream.str();
    }
}

#endif