cmake_minimum_required(VERSION 3.28)
project(ServerC__)

set(CMAKE_CXX_STANDARD 26)

add_executable(ServerC__ main.cpp
        FastAPI_CPP/http_lib.h
        FastAPI_CPP/FastAPI_CPP.h
        FastAPI_CPP/FastAPI_CPP.cpp)
