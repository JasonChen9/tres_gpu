file(GLOB_RECURSE TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

include_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${THIRD_PARTY_DIR}/tabulate/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)
link_libraries(nvidia-ml)

find_package(fmt 3.17 HINTS ${THIRD_PARTY_DIR}/fmt)
find_package(GTest CONFIG REQUIRED HINTS ${THIRD_PARTY_DIR}/google-test)

link_directories(${THIRD_PARTY_DIR}/google-test/lib)
link_libraries(gmock)
#enable_testing()
add_executable(tres_gather_test
        ${TEST_SRC}
        ../src/tres_gather_nvml.cpp
        ../src/tres_nvml.cpp)

target_link_libraries(tres_gather_test
                    fmt::fmt
                    GTest::gtest
                    GTest::gtest_main)
add_test(NAME tres_gather_test COMMAND test/tres_gather_test)

#include(GoogleTest)
#gtest_discover_tests(tres_gather_test)