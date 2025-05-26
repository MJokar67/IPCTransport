add_library(ipc_shared_memory
    include/SharedMemoryTransport.hpp
    src/SharedMemoryTransport.cxx
)
target_include_directories(ipc_shared_memory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ipc_shared_memory PRIVATE ipc_base)
