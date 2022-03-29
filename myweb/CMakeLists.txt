cmake_minimum_required(VERSION 3.16.3)
project(MyWebServer)

set(CMAKE_CXX_STANDARD 14)
set(SOURCES
        main.cpp
        ./myserver/webserver.cpp
        ./myhttp/myhttp.cpp
        ./mylistener/listener.cpp
        ./mytcpsession/tcpserver.cpp
        ./mytimer/mytimer.cpp
        )

include_directories(
        myhttp mylistener myserver mytcpsession mythread mytimer
)

add_executable(MyWebServer ${SOURCES})

target_link_libraries(MyWebServer pthread)
