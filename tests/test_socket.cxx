#include <IIPCTransport.hpp>
#include <IPCTransportFactory.hpp>
#include <TCPSocketTransport.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

TEST(IPC_PingPong, TCPSocket) {
  const std::string addr = "127.0.0.1:54321";

  pid_t pid = fork();
  ASSERT_GE(pid, 0) << "fork failed";

  if (pid == 0) {
    // Child process - client
    ipc::TCPSocketTransport client;
    ASSERT_TRUE(client.initialize(addr, false)) << "Child failed to connect";

    ipc::IPCMessage msg{};
    for (int i = 0; i < 5; ++i) {
      // Receive from parent
      ASSERT_TRUE(client.receive_message(msg)) << "Child failed to receive";
      std::cout << "[Child] Received: " << msg.counter << std::endl;

      // Increment counter and send back
      msg.counter++;
      ASSERT_TRUE(client.send_message(msg)) << "Child failed to send";
      std::cout << "[Child] Sent: " << msg.counter << std::endl;
    }

    client.cleanup();
    exit(0);
  } else {
    // Parent process - server
    ipc::TCPSocketTransport server;
    ASSERT_TRUE(server.initialize(addr, true))
        << "Parent failed to initialize server";

    ipc::IPCMessage msg{};
    msg.counter = 0;

    for (int i = 0; i < 5; ++i) {
      // Send to child
      msg.counter += 1;
      ASSERT_TRUE(server.send_message(msg)) << "Parent failed to send";
      std::cout << "[Parent] Sent: " << msg.counter << std::endl;

      // Receive from child
      ASSERT_TRUE(server.receive_message(msg)) << "Parent failed to receive";
      std::cout << "[Parent] Received: " << msg.counter << std::endl;
    }

    server.cleanup();

    int status = 0;
    waitpid(pid, &status, 0);
    ASSERT_TRUE(WIFEXITED(status)) << "Child did not exit normally";
  }
}
