// Tomas Costantino

#include <iostream>
#include <string>
#include <functional>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "http_lib.h"

const int PORT = 8080;
const int BUFFER_SIZE = 4096;

// Define route handlers
http::Response handle_get_users(const http::Request& req) {
    return {
            {1, 1}, // HTTP version 1.1
            200,
            "OK",
            {{"Content-Type", "application/json"}},
            "[{\"id\": 1, \"name\": \"John Doe\"}, {\"id\": 2, \"name\": \"Jane Doe\"}]"
    };
}

http::Response handle_post_user(const http::Request& req) {
    // In a real application, you would parse the request body and create a new user
    return {
            {1, 1},
            201,
            "Created",
            {{"Content-Type", "application/json"}},
            "{\"id\": 3, \"name\": \"New User\"}"
    };
}

http::Response handle_not_found(const http::Request& req) {
    return {
            {1, 1},
            404,
            "Not Found",
            {{"Content-Type", "text/plain"}},
            "Not Found"
    };
}

// Router function to map requests to handlers
http::Response route_request(const http::Request& req) {
    if (req.method == http::Method::GET && req.uri == "/api/users") {
        return handle_get_users(req);
    } else if (req.method == http::Method::POST && req.uri == "/api/users") {
        return handle_post_user(req);
    } else {
        return handle_not_found(req);
    }
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }

    std::cout << "Server listening on port " << PORT << std::endl;

    while (true) {
        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            continue;
        }

        // Read incoming request
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        std::string request_str(buffer);

        // Parse the request using our custom HTTP library
        http::Request req = http::parse_request(request_str);

        // Route the request and get the response
        http::Response resp = route_request(req);

        // Construct the response using our custom HTTP library
        std::string response_str = http::construct_response(resp);

        // Send the response
        send(new_socket, response_str.c_str(), response_str.length(), 0);

        close(new_socket);
    }

    return 0;
}