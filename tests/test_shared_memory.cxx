#include <IIPCTransport.hpp>
#include <IPCTransportFactory.hpp>
#include <SharedMemoryTransport.hpp>
#include <fcntl.h>
#include <gtest/gtest.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

const char *SEM_PARENT = "/test_sem_parent";
const char *SEM_CHILD = "/test_sem_child";
const std::string SHM_NAME = "/test_ipc_shm_ipcmsg";

TEST(IPC_PingPong, SharedMemory) {
  // Cleanup semaphores
  sem_unlink(SEM_PARENT);
  sem_unlink(SEM_CHILD);

  sem_t *sem_parent = sem_open(SEM_PARENT, O_CREAT, 0666, 0);
  sem_t *sem_child = sem_open(SEM_CHILD, O_CREAT, 0666, 0);
  ASSERT_NE(sem_parent, SEM_FAILED);
  ASSERT_NE(sem_child, SEM_FAILED);

  ipc::SharedMemoryTransport parentTransport;
  ASSERT_TRUE(parentTransport.initialize(SHM_NAME, true));

  pid_t pid = fork();
  ASSERT_NE(pid, -1);

  if (pid == 0) {
    // Child process
    ipc::SharedMemoryTransport childTransport;
    ASSERT_TRUE(childTransport.initialize(SHM_NAME, false));

    auto shared_msg_ptr = childTransport.get_shared_message();
    ASSERT_NE(shared_msg_ptr, nullptr);
    auto &msg = *shared_msg_ptr;

    while (!shared_msg_ptr->terminate) {
      sem_wait(sem_child);
      childTransport.receive_message(msg);
      std::cout << "[Child] Received: " << msg.counter << std::endl;

      if (msg.counter >= 10) {
        shared_msg_ptr->terminate = true;
        sem_post(sem_parent);
        break;
      }

      msg.counter++;
      snprintf(msg.data, sizeof(msg.data), "Child sent %u", msg.counter);
      childTransport.send_message(msg);
      std::cout << "[Child] Sent: " << msg.counter << std::endl;
      sem_post(sem_parent);
    }

    childTransport.cleanup();
    sem_close(sem_parent);
    sem_close(sem_child);
    _exit(0);
  } else {
    // Parent process
    ipc::SharedMemoryTransport parentTransport;
    ASSERT_TRUE(parentTransport.initialize(SHM_NAME, true));

    auto shared_msg_ptr = parentTransport.get_shared_message();
    ASSERT_NE(shared_msg_ptr, nullptr);
    auto &msg = *shared_msg_ptr;

    msg.counter = 0;
    snprintf(msg.data, sizeof(msg.data), "Parent sent %u", msg.counter);
    parentTransport.send_message(msg);
    std::cout << "[Parent] Sent: " << msg.counter << std::endl;
    sem_post(sem_child);

    while (msg.counter < 10 && !shared_msg_ptr->terminate) {
      sem_wait(sem_parent);
      parentTransport.receive_message(msg);
      std::cout << "[Parent] Received: " << msg.counter << std::endl;

      if (msg.counter >= 10) {
        shared_msg_ptr->terminate = true;
        sem_post(sem_child);
        break;
      }

      msg.counter++;
      snprintf(msg.data, sizeof(msg.data), "Parent sent %u", msg.counter);
      parentTransport.send_message(msg);
      std::cout << "[Parent] Sent: " << msg.counter << std::endl;
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