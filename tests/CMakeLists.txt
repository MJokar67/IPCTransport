enable_testing()

include(GoogleTest)

include(FetchContent)

# Collect all test source files
set(TEST_SOURCES
  test_main.cxx
  test_pipe.cxx
  test_shared_memory.cxx
  test_socket.cxx
  test_message_queue.cxx
  # test_signal.cxx
)

add_executable(ipc_tests ${TEST_SOURCES})

# Link IPC base and all ipc modules, plus gtest libraries
target_link_libraries(ipc_tests
  PRIVATE
  ipc_pipe
  ipc_factory
  gtest_main
)

include(GoogleTest)
gtest_discover_tests(ipc_tests)
