// ipc/shared_memory/SharedMemoryTransport.cpp
#include <SharedMemoryTransport.hpp>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

ipc::SharedMemoryTransport::SharedMemoryTransport() {}

ipc::SharedMemoryTransport::~SharedMemoryTransport() { cleanup(); }

bool ipc::SharedMemoryTransport::initialize(const std::string &name,
  bool create) {
  shm_name = "/" + name;

  if (create) {
    shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
      perror("shm_open create");
      return false;
    }

    if (ftruncate(shm_fd, sizeof(IPCMessageSHM)) == -1) {
      perror("ftruncate");
      return false;
    }

    is_owner = true;
  } else {
    shm_fd = shm_open(shm_name.c_str(), O_RDWR, 0666);
    if (shm_fd == -1) {
      perror("shm_open open");
      return false;
    }
  }

  void *ptr = mmap(nullptr, sizeof(IPCMessageSHM), PROT_READ | PROT_WRITE,
  MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap");
    return false;
  }

  shared_msg = static_cast<IPCMessageSHM *>(ptr);

  if (create) {
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared_msg->mutex, &mattr);
    pthread_mutexattr_destroy(&mattr);

    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    pthread_cond_init(&shared_msg->cond, &cattr);
    pthread_condattr_destroy(&cattr);

    shared_msg->counter = 0;
    shared_msg->ready = false;
    shared_msg->finished = false;
    memset(shared_msg->data, 0, sizeof(shared_msg->data));
  }

  return true;
}

bool ipc::SharedMemoryTransport::send_message(const IPCMessage &msg) {
  pthread_mutex_lock(&shared_msg->mutex);
  while (shared_msg->ready) {
    pthread_cond_wait(&shared_msg->cond, &shared_msg->mutex);
  }

  shared_msg->counter = msg.counter;
  shared_msg->finished = msg.finished;
  strncpy(shared_msg->data, msg.data, sizeof(shared_msg->data));
  shared_msg->ready = true;

  pthread_cond_signal(&shared_msg->cond);
  pthread_mutex_unlock(&shared_msg->mutex);

  return true;
}

bool ipc::SharedMemoryTransport::receive_message(IPCMessage &msg) {
  pthread_mutex_lock(&shared_msg->mutex);
  while (!shared_msg->ready) {
    pthread_cond_wait(&shared_msg->cond, &shared_msg->mutex);
  }

  msg.counter = shared_msg->counter;
  msg.finished = shared_msg->finished;
  strncpy(msg.data, shared_msg->data, sizeof(msg.data));
  shared_msg->ready = false;

  pthread_cond_signal(&shared_msg->cond);
  pthread_mutex_unlock(&shared_msg->mutex);

  return true;
}

void ipc::SharedMemoryTransport::cleanup() {
  if (shared_msg) {
    munmap(shared_msg, sizeof(IPCMessage));
    shared_msg = nullptr;
  }

  if (shm_fd != -1) {
    close(shm_fd);
    shm_fd = -1;
  }

  if (is_owner) {
    shm_unlink(shm_name.c_str());
  }
}

ipc::IPCMessageSHM *ipc::SharedMemoryTransport::get_shared_message() const {
  return shared_msg;
}
