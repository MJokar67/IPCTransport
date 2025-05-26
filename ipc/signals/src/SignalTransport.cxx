#include <SignalTransport.hpp>
#include <cerrno>
#include <csignal>
#include <iostream>

std::atomic<bool> ipc::SignalTransport::signal_received{false};

ipc::SignalTransport::SignalTransport() = default;

ipc::SignalTransport::~SignalTransport() { cleanup(); }

bool ipc::SignalTransport::initialize(const std::string &name, bool create) {
  shm_name = "/" + name;

  if (create) {
    shm_fd = shm_open(shm_name.c_str(), O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
      perror("shm_open");
      return false;
    }
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
      perror("ftruncate");
      return false;
    }
  } else {
    shm_fd = shm_open(shm_name.c_str(), O_RDWR, 0666);
    if (shm_fd == -1) {
      perror("shm_open");
      return false;
    }
  }

  shared_msg = (IPCMessage *)mmap(nullptr, SHM_SIZE, PROT_READ | PROT_WRITE,
                                  MAP_SHARED, shm_fd, 0);
  if (shared_msg == MAP_FAILED) {
    perror("mmap");
    return false;
  }

  struct sigaction sa {};
  sa.sa_handler = SignalTransport::signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction(SIGUSR1, &sa, nullptr) == -1) {
    perror("sigaction");
    return false;
  }

  signal_received = false;
  return true;
}

bool ipc::SignalTransport::send_message(const IPCMessage &msg) {
  if (!shared_msg)
    return false;

  std::memcpy(shared_msg, &msg, sizeof(IPCMessage));

  if (peer_pid > 0) {
    if (kill(peer_pid, SIGUSR1) == -1) {
      perror("kill");
      return false;
    }
  } else {
    std::cerr << "Peer PID not set\n";
    return false;
  }
  return true;
}

bool ipc::SignalTransport::wait_for_signal() {
  sigset_t waitset;
  sigemptyset(&waitset);
  sigaddset(&waitset, SIGUSR1);

  // Block SIGUSR1 and wait synchronously
  int sig = 0;
  siginfo_t info;
  while (true) {
    sig = sigwaitinfo(&waitset, &info);
    if (sig == SIGUSR1) {
      return true;
    } else if (sig == -1 && errno == EINTR) {
      continue; // interrupted, try again
    } else {
      perror("sigwaitinfo");
      return false;
    }
  }
}

bool ipc::SignalTransport::receive_message(IPCMessage &msg) {
  if (!shared_msg)
    return false;

  if (!wait_for_signal())
    return false;

  std::memcpy(&msg, shared_msg, sizeof(IPCMessage));
  return true;
}

void ipc::SignalTransport::cleanup() {
  if (shared_msg) {
    munmap(shared_msg, SHM_SIZE);
    shared_msg = nullptr;
  }
  if (shm_fd != -1) {
    close(shm_fd);
    shm_fd = -1;
  }
  shm_unlink(shm_name.c_str());
}

void ipc::SignalTransport::signal_handler(int) { signal_received = true; }

void ipc::SignalTransport::setPeerPid(pid_t pid) { peer_pid = pid; }
