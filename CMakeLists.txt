cmake_minimum_required(VERSION 3.10)

project(epoll_server)  

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11")

set(SRC 
)

add_executable(EpollServer src/EpollServer.cpp)
add_executable(client src/client.cpp)
