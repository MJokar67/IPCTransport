#include <MsgQueueTransport.hpp>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <cassert>


ipc::MsgQueueTransport::MsgQueueTransport() = default;

ipc::MsgQueueTransport::~MsgQueueTransport() { cleanup(); }

bool ipc::MsgQueueTransport::initialize(const std::string &name, bool create) {

  struct mq_attr attr;

  attr.mq_flags = 0;

  attr.mq_maxmsg = 10;

  attr.mq_msgsize = sizeof(ipc::IPCMessage);

  attr.mq_curmsgs = 0;

  auto CHILD_TO_PARENT_QUEUE = "_ctp";
  auto PARENT_TO_CHILD_QUEUE = "_ptc";

  if(create) {
    send_name = name + CHILD_TO_PARENT_QUEUE;
    recieve_name = name + PARENT_TO_CHILD_QUEUE;

    send_mq = mq_open(send_name.c_str(), O_WRONLY);
    recieve_mq = mq_open(recieve_name.c_str(), O_RDONLY);
  } else {
    send_name = name + PARENT_TO_CHILD_QUEUE;
    recieve_name = name + CHILD_TO_PARENT_QUEUE;
    
    send_mq = mq_open(send_name.c_str(), O_WRONLY);
    recieve_mq = mq_open(recieve_name.c_str(), O_RDONLY);
  }

  if (send_mq == (mqd_t)-1) {
    perror("mq_open failed for send_mq object");
    return false;
  }

  if (recieve_mq == (mqd_t)-1) {
    perror("mq_open failed for send_mq recieve_mq");
    return false;
  }

  return true;
}

bool ipc::MsgQueueTransport::send_message(const IPCMessage &msg) {
  // Use msg.counter as message priority
  return mq_send(send_mq, (const char *)&msg, sizeof(msg), 0) == 0;
}

bool ipc::MsgQueueTransport::receive_message(IPCMessage &msg) {
  auto received = mq_receive(recieve_mq, (char *)&msg, sizeof(msg), nullptr);
  if (received < 0) {
    perror("mq_receive failed");
    return false;
  }
  return true;
}

void ipc::MsgQueueTransport::cleanup() {
  if (send_mq != -1) {
    mq_close(send_mq);
    if (!send_name.empty()) {
      mq_unlink(send_name.c_str());
    }
    send_mq = -1;
  }
  if (recieve_mq != -1) {
    mq_close(recieve_mq);
    if (!recieve_name.empty()) {
      mq_unlink(recieve_name.c_str());
    }
    recieve_mq = -1;
  }
}