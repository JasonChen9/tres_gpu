set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

include_directories(${CMAKE_SOURCE_DIR}/lib)
link_directories(${CMAKE_SOURCE_DIR}/lib)

link_libraries(nvidia-ml)
link_libraries(cuda)

include_directories(${THIRD_PARTY_DIR}/tabulate/include)
find_package(fmt 3.17 HINTS ${THIRD_PARTY_DIR}/fmt)

set(ROOT tres_gather.cpp)
set(NVML tres_gather_nvml.cpp tres_nvml.cpp)
set(CGROUP tres_gather_cgroup.cpp)
set(CUDA tres_gather_cuda.cpp)

add_executable(main main.cpp ${ROOT} ${NVML} ${CGROUP} ${CUDA})

target_link_libraries(main fmt::fmt)
