cmake_minimum_required(VERSION 3.15)
project(IPCTransport CXX)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(ipc)

add_executable(main src/main.cxx)
target_link_libraries(main PRIVATE ipc_base ipc_shared_memory)
