cmake_minimum_required(VERSION 3.16.3)
project(HelloServer)

set(CMAKE_CXX_STANDARD 14)
set(SOURCES
        main.cpp
        ./webserver/webserver.cpp
        ./http/http_conn.cpp
        ./timer/Timer.cpp
        ./Utils/Utils.cpp
        )

include_directories(
        webserver http lock threadpool timer Utils
        )

add_executable(HelloServer ${SOURCES})

target_link_libraries(HelloServer pthread)
