cmake_minimum_required(VERSION 3.15)
project(IPCTransport CXX)

set(CMAKE_CXX_STANDARD 17)


# Add GoogleTest from submodule
add_subdirectory(external/googletest)

add_subdirectory(ipc)
add_subdirectory(tests)

add_executable(main src/main.cxx)
target_link_libraries(main PRIVATE ipc_base ipc_shared_memory)
