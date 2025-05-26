#include <IIPCTransport.hpp>
#include <IPCTransportFactory.hpp>
#include <MsgQueueTransport.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

constexpr const char *queue_name = "pingpong_queue";
constexpr int MAX_COUNT = 10;

TEST(IPC_PingPong, MessageQueue) {
  pid_t pid = fork();
  ASSERT_NE(pid, -1) << "fork() failed";

  if (pid == 0) {
    // Child process
    ipc::MsgQueueTransport client;
    if (!client.initialize(queue_name, false)) {
      std::cerr << "Child failed to open message queue" << std::endl;
      exit(1);
    }

    while (true) {
      ipc::IPCMessage msg;
      client.receive_message(msg);

      std::cout << "[Child] Received: " << msg.counter << std::endl;

      if (msg.counter > MAX_COUNT)
        break;

      msg.counter += 1;
      msg.data[0] = 'C';
      client.send_message(msg);

      std::cout << "[Child] Sent: " << msg.counter << std::endl;
    }
    exit(0);
  } else {
    // Parent process
    ipc::MsgQueueTransport server;
    ASSERT_TRUE(server.initialize(queue_name, true))
        << "Parent failed to create message queue";

    ipc::IPCMessage msg;
    msg.counter = 0;
    msg.data[0] = 'P';

    server.send_message(msg);
    std::cout << "[Parent] Sent: " << msg.counter << std::endl;

    while (true) {
      ipc::IPCMessage response;
      server.receive_message(response);

      std::cout << "[Parent] Received: " << response.counter << std::endl;

      if (response.counter > MAX_COUNT) {
        // Notify child to terminate
        response.counter += 1;
        response.data[0] = 'P';
        server.send_message(response);
        break;
      }

      EXPECT_EQ(response.counter, msg.counter + 1)
          << "Parent expected child to increment counter";

      msg = response;
      msg.counter += 1;
      msg.data[0] = 'P';

      server.send_message(msg);
      std::cout << "[Parent] Sent: " << msg.counter << std::endl;
    }

    waitpid(pid, nullptr, 0);
  }
}
