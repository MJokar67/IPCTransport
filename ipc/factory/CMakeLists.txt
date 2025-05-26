add_library(ipc_factory STATIC
    include/IPCTransportFactory.hpp
    src/IPCTransportFactory.cxx
)

target_include_directories(ipc_factory PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ipc_factory
    PUBLIC ipc_base
           ipc_pipe
           ipc_shared_memory
           ipc_socket
           ipc_msgqueue
)
