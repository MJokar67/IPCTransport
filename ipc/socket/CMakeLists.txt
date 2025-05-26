add_library(ipc_socket
    include/TCPSocketTransport.hpp
    src/TCPSocketTransport.cxx
)
target_include_directories(ipc_socket PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ipc_socket PRIVATE ipc_base)
