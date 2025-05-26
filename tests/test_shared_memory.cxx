#include <IIPCTransport.hpp>
#include <IPCTransportFactory.hpp>
#include <SharedMemoryTransport.hpp>
#include <fcntl.h> // For O_CREAT, O_EXCL
#include <gtest/gtest.h>
#include <semaphore.h>
#include <sys/stat.h> // For mode constants
#include <unistd.h>

const char *SEM_PARENT = "/test_sem_parent";
const char *SEM_CHILD = "/test_sem_child";
const std::string SHM_NAME = "/test_ipc_shm_ipcmsg";

TEST(IPC_PingPong, SharedMemory) {
  // Cleanup semaphores before start
  sem_unlink(SEM_PARENT);
  sem_unlink(SEM_CHILD);

  sem_t *sem_parent = sem_open(SEM_PARENT, O_CREAT, 0666, 1); // Parent starts
  sem_t *sem_child = sem_open(SEM_CHILD, O_CREAT, 0666, 0);   // Child waits
  ASSERT_NE(sem_parent, SEM_FAILED) << "sem_open parent failed";
  ASSERT_NE(sem_child, SEM_FAILED) << "sem_open child failed";

  ipc::SharedMemoryTransport parentTransport;
  ASSERT_TRUE(parentTransport.initialize(SHM_NAME, true))
      << "Parent failed to init shared memory";

  pid_t pid = fork();
  ASSERT_NE(pid, -1) << "fork failed";

  if (pid == 0) {
    // Child process
    ipc::SharedMemoryTransport childTransport;
    if (!childTransport.initialize(SHM_NAME, false)) {
      std::cerr << "Child failed to init shared memory\n";
      _exit(1);
    }

    ipc::IPCMessage *msg = childTransport.get_shared_message();
    ASSERT_NE(msg, nullptr);

    while (true) {
      sem_wait(sem_child);

      uint32_t val = msg->counter;
      std::cout << "[Child] Received: " << val << std::endl;

      if (val >= 10) {
        sem_post(sem_parent);
        break;
      }

      msg->counter = val + 1;
      snprintf(msg->data, sizeof(msg->data), "Child sent %u", msg->counter);
      std::cout << "[Child] Sent: " << msg->counter << std::endl;

      sem_post(sem_parent);
    }

    childTransport.cleanup();
    sem_close(sem_parent);
    sem_close(sem_child);
    _exit(0);
  } else {
    // Parent process
    ipc::IPCMessage *msg = parentTransport.get_shared_message();
    ASSERT_NE(msg, nullptr);

    // Initialize message
    msg->counter = 0;
    snprintf(msg->data, sizeof(msg->data), "Start");

    while (true) {
      sem_wait(sem_parent);

      uint32_t val = msg->counter;
      std::cout << "[Parent] Sent: " << val << std::endl;

      if (val >= 10) {
        sem_post(sem_child);
        break;
      }

      sem_post(sem_child);
    }

    wait(nullptr);

    parentTransport.cleanup();

    sem_close(sem_parent);
    sem_close(sem_child);
    sem_unlink(SEM_PARENT);
    sem_unlink(SEM_CHILD);
  }
}
