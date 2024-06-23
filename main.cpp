// Tomas Costantino
#include "FastAPI_CPP/FastAPI_CPP.h"

int main() {
    fastapi_cpp::FastAPI app;

    app.get("/", [](const fastapi_cpp::Request& request, const std::map<std::string, std::string>& params) {
        return http::HTTP_200_OK(http::JSON::object({{"message", "Welcome"}}));
    });

    app.get("/param_query", [](const fastapi_cpp::Request& request, const std::map<std::string, std::string>& params) {
        http::JSON::Object response_data;
        for (const auto& [key, value] : params) {
            response_data[key] = value;
        }
        return http::HTTP_200_OK(response_data);
    });

    app.get("/echo", [](const fastapi_cpp::Request& request, const std::map<std::string, std::string>& params) {
        return http::HTTP_200_OK(http::JSON::object({{"message", "Echo"}}));
    });

    app.get("/test", [](const fastapi_cpp::Request& request, const std::map<std::string, std::string>& params) {
        return http::HTTP_200_OK(http::JSON::object({{"message", "Testing"}}));
    });

    app.get("/echo/{echo}", [](const fastapi_cpp::Request& request, const std::map<std::string, std::string>& params) {
        auto to_echo = params.at("echo");
        return http::HTTP_200_OK(http::JSON::object({{"Echo route", to_echo}}));
    });

    app.run(8000);

    return 0;
}