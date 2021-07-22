file(GLOB_RECURSE TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
add_executable(tres_gather_test
        ${TEST_SRC}
        ../src/TresNvml.cpp
        ../src/TresGather.cpp)

add_executable(test_task
        hello.cu)
set_target_properties(test_task
        PROPERTIES CUDA_ARCHITECTURES "35;50;72"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/generate)

target_link_libraries(tres_gather_test
        fmt::fmt
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
        tabulate
        nvidia-ml
        libcuda.so.7
        )

add_test(NAME tres_gather_test COMMAND test/tres_gather_test)