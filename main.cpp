// Tomas Costantino

#include "FastAPI_CPP/FastAPI_CPP.h"

int main () {
    fastapi_cpp::FastAPI app;

    app.get("/", [](const fastapi_cpp::Request& request) {
        return fastapi_cpp::Response{
                {1, 1},
                200,
                "OK",
                {{"Content-Type", "application/json"}},
                "Welcome"
        };
    });

    app.run(8000);

    return 0;
}
