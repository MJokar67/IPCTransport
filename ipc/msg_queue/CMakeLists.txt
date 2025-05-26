add_library(ipc_msgqueue
    include/MsgQueueTransport.hpp
    src/MsgQueueTransport.cxx
)
target_include_directories(ipc_msgqueue PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ipc_msgqueue PRIVATE ipc_base)
