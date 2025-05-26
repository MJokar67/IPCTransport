#include <IIPCTransport.hpp>
#include <IPCTransportFactory.hpp>
#include <PipeTransport.hpp>
#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace std;

TEST(IPC_PingPong, Pipe) {
  const std::string ipc_name = "test_pipe_ipc";

  pid_t pid = fork();
  ASSERT_NE(pid, -1) << "fork failed";

  if (pid == 0) {
    // Child process: receive -> increment -> send
    ipc::PipeTransport transport;
    bool init = transport.initialize(ipc_name, false);
    ASSERT_TRUE(init) << "Child failed to initialize PipeTransport";

    ipc::IPCMessage msg{};
    while (true) {
      ASSERT_TRUE(transport.receive_message(msg));
      cout << "[Child] Received: " << msg.counter << endl;

      if (msg.counter >= 10)
        break;

      // Increment and send back
      msg.counter++;
      ASSERT_TRUE(transport.send_message(msg));
      cout << "[Child] Sent: " << msg.counter << endl;
    }

    transport.cleanup();
    _exit(0);
  } else {
    // Parent process: send -> receive -> send ...
    ipc::PipeTransport transport;
    bool init = transport.initialize(ipc_name, true);
    ASSERT_TRUE(init) << "Parent failed to initialize PipeTransport";

    ipc::IPCMessage msg{};
    msg.counter = 0;

    // Start ping-pong by sending 0
    ASSERT_TRUE(transport.send_message(msg));
    cout << "[Parent] Sent: " << msg.counter << endl;

    while (true) {
      ASSERT_TRUE(transport.receive_message(msg));
      cout << "[Parent] Received: " << msg.counter << endl;

      if (msg.counter >= 10)
        break;

      // Increment and send back
      msg.counter++;
      ASSERT_TRUE(transport.send_message(msg));
      cout << "[Parent] Sent: " << msg.counter << endl;
    }

    int status = 0;
    waitpid(pid, &status, 0);

    transport.cleanup();

    ASSERT_TRUE(WIFEXITED(status));
    ASSERT_EQ(WEXITSTATUS(status), 0);
  }
}
