add_library(ipc_pipe
    include/PipeTransport.hpp
    src/PipeTransport.cxx
)
target_include_directories(ipc_pipe PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ipc_pipe PRIVATE ipc_base)
