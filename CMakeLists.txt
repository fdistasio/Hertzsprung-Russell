cmake_minimum_required(VERSION 3.10)

project(HrDiagram VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3  -march=native -flto -fopenmp")

include_directories(${CMAKE_SOURCE_DIR}/include)

set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)
set(SOURCES ${SRC_DIR}/hr.cpp ${SRC_DIR}/star.cpp ${SRC_DIR}/parse_consume.cpp)

add_executable(hr ${SOURCES})

find_package(Boost REQUIRED COMPONENTS system filesystem iostreams)

target_link_libraries(hr PRIVATE Boost::system Boost::filesystem Boost::iostreams)

# time ./hr ../data/hygdata_v41.csv 14 16
# time ./hr ../data/athyg_24_reduced_m11.csv 21 22
# ./hr ../data/hygdata_v41.csv 14 16
# ./hr ../data/athyg_24_reduced_m11.csv 21 22