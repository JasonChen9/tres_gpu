cmake_minimum_required(VERSION 3.17)
#SET(CMAKE_CXX_FLAGS "-fsanitize=address")
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)
project(tres)

find_package(fmt 3.17 HINTS ${THIRD_PARTY_DIR}/fmt)
find_package(GTest CONFIG REQUIRED HINTS ${THIRD_PARTY_DIR}/google-test)

add_library(tabulate INTERFACE)
target_include_directories(tabulate INTERFACE ${THIRD_PARTY_DIR}/tabulate/include)

add_library(nvidia-ml SHARED IMPORTED)
set_property(TARGET nvidia-ml PROPERTY IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/lib/libnvidia-ml.so")

enable_language(CUDA)

add_subdirectory(src)
add_subdirectory(test)

