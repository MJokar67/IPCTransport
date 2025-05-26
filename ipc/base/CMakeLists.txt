
add_library(ipc_base INTERFACE
    include/IIPCTransport.hpp
    include/IIPCMessage.hpp
)
target_include_directories(ipc_base INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)