cmake_minimum_required(VERSION 3.10)

project(simple-epoller)  

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c11")

set(SRC 
    src/Epoller.cpp
)

add_executable(EchoServer src/EchoServer.cpp ${SRC})
target_link_libraries(EchoServer pthread)

add_executable(EchoClient src/EchoClient.cpp ${SRC})
target_link_libraries(EchoClient pthread)

