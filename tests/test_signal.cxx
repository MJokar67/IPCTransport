#include <IIPCTransport.hpp>
#include <IPCTransportFactory.hpp>
#include <SignalTransport.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>

using namespace ipc;

constexpr const char *queue_name = "signal_transport_queue";
constexpr int MAX_COUNT = 10;

TEST(IPC_PingPong, SignalTransport) {
  pid_t pid = fork();
  ASSERT_NE(pid, -1) << "fork() failed";

  // Block SIGUSR1 before anything else in both processes
  sigset_t mask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  if (sigprocmask(SIG_BLOCK, &mask, nullptr) < 0) {
    perror("sigprocmask");
    ASSERT_TRUE(false) << "Failed to block SIGUSR1";
  }

  if (pid == 0) {
    // Child process
    ipc::SignalTransport client;
    ASSERT_TRUE(client.initialize(queue_name, false))
        << "Child failed to open shared memory";

    IPCMessage msg;

    // Receive handshake (parent PID)
    ASSERT_TRUE(client.receive_message(msg))
        << "Child failed to receive handshake";
    pid_t parent_pid = msg.counter;
    std::cout << "[Child] Parent PID received: " << parent_pid << std::endl;
    client.setPeerPid(parent_pid);

    // Send handshake back: child's pid
    msg.counter = getpid();
    ASSERT_TRUE(client.send_message(msg))
        << "Child failed to send handshake back";

    int counter = 0;

    while (true) {
      ASSERT_TRUE(client.receive_message(msg))
          << "Child failed to receive message";
      std::cout << "[Child] Received: " << msg.counter << std::endl;

      if (msg.counter >= MAX_COUNT) {
        std::cout << "[Child] Reached max count. Exiting.\n";
        break;
      }

      msg.counter += 1;
      msg.data[0] = 'C';

      ASSERT_TRUE(client.send_message(msg)) << "Child failed to send message";
      std::cout << "[Child] Sent: " << msg.counter << std::endl;
    }
    exit(0);

  } else {
    // Parent process
    ipc::SignalTransport server;
    ASSERT_TRUE(server.initialize(queue_name, true))
        << "Parent failed to create shared memory";

    server.setPeerPid(pid);

    // Send handshake: parent's pid
    IPCMessage handshake_msg;
    handshake_msg.counter = getpid();
    handshake_msg.data[0] = 'H';

    ASSERT_TRUE(server.send_message(handshake_msg))
        << "Parent failed handshake send";

    // Receive handshake back: child's pid
    IPCMessage msg;
    ASSERT_TRUE(server.receive_message(msg))
        << "Parent failed to receive handshake back";
    pid_t child_pid = msg.counter;
    std::cout << "[Parent] Child PID received: " << child_pid << std::endl;
    server.setPeerPid(child_pid);

    // Start ping-pong with counter=0
    msg.counter = 0;
    msg.data[0] = 'P';

    ASSERT_TRUE(server.send_message(msg)) << "Parent failed initial send";
    std::cout << "[Parent] Sent: " << msg.counter << std::endl;

    while (true) {
      ASSERT_TRUE(server.receive_message(msg))
          << "Parent failed to receive message";
      std::cout << "[Parent] Received: " << msg.counter << std::endl;

      if (msg.counter >= MAX_COUNT) {
        std::cout << "[Parent] Reached max count. Exiting.\n";
        break;
      }

      msg.counter += 1;
      msg.data[0] = 'P';

      ASSERT_TRUE(server.send_message(msg)) << "Parent failed to send message";
      std::cout << "[Parent] Sent: " << msg.counter << std::endl;
    }

    waitpid(pid, nullptr, 0);
  }
}
