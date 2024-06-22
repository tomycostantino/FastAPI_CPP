// Tomas Costantino
#include "FastAPI_CPP/FastAPI_CPP.h"

int main() {
    fastapi_cpp::FastAPI app;

    app.get("/", [](const fastapi_cpp::Request& request) {
        return http::HTTP_200_OK("Welcome", {{"Content-Type", "application/json"}});
    });

    app.get("/json", [](const fastapi_cpp::Request& request) {
        return http::HTTP_200_OK(http::JSON::object({{"field1", "value"}, {"field2", 2}}));
    });


    app.run(8000);

    return 0;
}