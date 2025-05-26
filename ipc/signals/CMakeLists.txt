add_library(ipc_signal
    include/SignalTransport.hpp
    src/SignalTransport.cxx
)
target_include_directories(ipc_signal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ipc_signal PRIVATE ipc_base)
