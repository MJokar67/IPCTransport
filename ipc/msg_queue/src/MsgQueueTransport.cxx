#include <MsgQueueTransport.hpp>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

constexpr size_t MAX_MSG_SIZE = sizeof(ipc::IPCMessage);
constexpr size_t MAX_MSG_COUNT = 10;
constexpr int PERMISSIONS = 0666;

ipc::MsgQueueTransport::MsgQueueTransport() = default;

ipc::MsgQueueTransport::~MsgQueueTransport() { cleanup(); }

bool ipc::MsgQueueTransport::initialize(const std::string &name, bool create) {
  queue_name = "/" + name; // POSIX queues require leading '/'
  struct mq_attr attr {};
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = sizeof(IPCMessage);
  attr.mq_curmsgs = 0;

  if (create) {
    mq_unlink(queue_name.c_str()); // Ensure clean start
    mq = mq_open(queue_name.c_str(), O_CREAT | O_RDWR, PERMISSIONS, &attr);
  } else {
    mq = mq_open(queue_name.c_str(), O_RDWR);
  }

  if (mq == (mqd_t)-1) {
    perror("mq_open failed");
    return false;
  }

  return true;
}

bool ipc::MsgQueueTransport::send_message(const IPCMessage &msg) {
  // Use msg.counter as message priority
  unsigned int priority = msg.counter;
  return mq_send(mq, reinterpret_cast<const char *>(&msg), sizeof(IPCMessage),
                 priority) == 0;
}

bool ipc::MsgQueueTransport::receive_message(IPCMessage &msg) {
  unsigned int priority;
  ssize_t received = mq_receive(mq, reinterpret_cast<char *>(&msg),
                                sizeof(IPCMessage), &priority);
  if (received < 0) {
    perror("mq_receive failed");
    return false;
  }
  return true;
}

void ipc::MsgQueueTransport::cleanup() {
  if (mq != -1) {
    mq_close(mq);
    if (!queue_name.empty()) {
      mq_unlink(queue_name.c_str());
    }
    mq = -1;
  }
}
