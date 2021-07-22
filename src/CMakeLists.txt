file(GLOB_RECURSE MAIN_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_executable(main ${MAIN_SRC})

target_link_libraries(main
        fmt::fmt
        tabulate
        nvidia-ml
        libcuda.so.7
        )

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR}/lib)
