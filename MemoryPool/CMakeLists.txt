cmake_minimum_required(VERSION 3.16.3)
project(MemPool)

set(CMAKE_CXX_STANDARD 14)

aux_source_directory(./src SOURCE)

include_directories(./include)

add_executable(MemPool ${SOURCE})

target_link_libraries(MemPool pthread)