// Tomas Costantino

#include "FastAPI_CPP.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>

namespace fastapi_cpp {
    FastAPI* FastAPI::instance = nullptr;

    FastAPI::FastAPI() : running(false), server_fd(-1) {
        instance = this;
    }

    FastAPI::~FastAPI() {
        stop();
    }

    void FastAPI::signal_handler(int signal) {
        std::cout << "Received signal " << signal << ". Shutting down..." << std::endl;
        if (instance) {
            instance->stop();
        }
    }

    Response FastAPI::handle_request(const Request &req) {
        for (const auto& route : routes) {
            Response response = route->handle(req);
            if (response.status_code != 404) {
                return response;
            }
        }
        return {{1, 1}, 404, "Not Found", {{"Content-Type", "text/plain"}}, "Not Found"};
    }

    void FastAPI::run(int port) {
        int server_fd, new_socket;
        struct sockaddr_in address;
        int addrlen = sizeof(address);
        char buffer[4096] = {0};

        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
            throw std::runtime_error("Socket creation failed");
        }

        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
            close(server_fd);
            throw std::runtime_error("Bind failed");
        }

        if (listen(server_fd, 3) < 0) {
            close(server_fd);
            throw std::runtime_error("Listen failed");
        }

        std::cout << "Server listening on port " << port << std::endl;

        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);

        running = true;

        while (running) {
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(server_fd, &readfds);

            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            int activity = select(server_fd + 1, &readfds, NULL, NULL, &timeout);

            if (activity < 0 && errno != EINTR) {
                std::cerr << "Select error" << std::endl;
                continue;
            }

            if (!running) break;

            if (activity == 0) continue;

            int new_socket;
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }

            char buffer[4096] = {0};
            int valread = read(new_socket, buffer, 4096);
            if (valread < 0) {
                std::cerr << "Read failed" << std::endl;
                close(new_socket);
                continue;
            }

            std::string request_str(buffer);
            std::cout << "Received request:\n" << request_str << std::endl;

            try {
                Request req = http::parse_request(request_str);
                Response resp = handle_request(req);
                std::string response_str = http::construct_response(resp);

                std::cout << "Sending response:\n" << response_str << std::endl;

                if (send(new_socket, response_str.c_str(), response_str.length(), 0) < 0) {
                    std::cerr << "Send failed" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error handling request: " << e.what() << std::endl;
            }

            close(new_socket);
        }

        std::cout << "Server stopped" << std::endl;
    }

    void FastAPI::stop() {
        running = false;
        if (server_fd != -1) {
            close(server_fd);
            server_fd = -1;
        }
    }
}
