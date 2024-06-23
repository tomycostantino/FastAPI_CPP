// Tomas Costantino
#include "FastAPI_CPP/FastAPI_CPP.h"

int main() {
    fastapi_cpp::FastAPI app;

    app.get("/", [](const fastapi_cpp::Request& request, const std::map<std::string, std::string>& params) {
        return http::HTTP_200_OK(http::JSON::object({{"message", "Welcome"}}));
    });

    app.get("/echo", [](const fastapi_cpp::Request& request, const std::map<std::string, std::string>& params) {
        return http::HTTP_200_OK(http::JSON::object({{"message", "Echo"}}));
    });

    app.get("/test", [](const fastapi_cpp::Request& request, const std::map<std::string, std::string>& params) {
        return http::HTTP_200_OK(http::JSON::object({{"message", "Testing"}}));
    });

    app.get("/echo/{echo}", [](const fastapi_cpp::Request& request, const std::map<std::string, std::string>& params) {
        auto route = params.at("echo");
        return http::HTTP_200_OK(http::JSON::object({{"Echo route", route}}));
    });

    app.run(8000);

    return 0;
}