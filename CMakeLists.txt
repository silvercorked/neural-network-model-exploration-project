cmake_minimum_required(VERSION 3.15)
project(MLInputGenerator)
set(CMAKE_CXX_STANDARD 20)

include_directories(include)
include_directories(include/concepts)
include_directories(include/core)
include_directories(include/core/abstract)
include_directories(include/programs)
include_directories(include/util)

# dependencies
include_directories(/usr/local/include/fplus)   # functional plus
include_directories(/usr/local/include/eigen)   # eigen
include_directories(/usr/local/include/json)    # json
include_directories(/usr/local/include/fdeep)   # frugally-deep

#set(SOURCES main.cpp)

add_executable(MLInputGenerator src/main.cpp)
