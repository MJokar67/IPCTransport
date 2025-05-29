#include <IIPCTransport.hpp>
#include <IPCTransportFactory.hpp>
#include <MsgQueueTransport.hpp>
#include <IPCTransportFactory.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

#include <gtest/gtest.h>

#include <iostream>

#include <string>

#include <unistd.h>

#include <mqueue.h>

#include <fcntl.h>

#include <sys/wait.h>

#include <cerrno>

const std::string QUEUE_BASE_NAME {"/queue"};


const int MAX_COUNT = 10;

TEST(IPC_PingPong, MessageQueue) {

  // Cleanup
  mq_unlink((QUEUE_BASE_NAME + "_ctp").c_str());
  mq_unlink((QUEUE_BASE_NAME + "_ptc").c_str());
  
  // Initial the files at the begin of the test.
  {
    struct mq_attr attr;

    attr.mq_flags = 0;

    attr.mq_maxmsg = 10;

    attr.mq_msgsize = sizeof(ipc::IPCMessage);

    attr.mq_curmsgs = 0;

    mqd_t p2c_mq = mq_open((QUEUE_BASE_NAME + "_ctp").c_str(), O_CREAT | O_RDWR, 0666, &attr);

    ASSERT_NE(p2c_mq, (mqd_t)-1);

    mqd_t c2p_mq = mq_open((QUEUE_BASE_NAME + "_ptc").c_str(), O_CREAT | O_RDWR, 0666, &attr);

    ASSERT_NE(c2p_mq, (mqd_t)-1);
  }

  pid_t pid = fork();

  ASSERT_NE(pid, -1);

  if (pid == 0) {

    // Child process

    auto transport = IPCTransportFactory::create_transport(IPCType::MessageQueue);

    transport->initialize(QUEUE_BASE_NAME, true);

    ipc::IPCMessage msg_recv;

    while (true) {

      if(transport->receive_message(msg_recv)) {
        if (msg_recv.counter == -1) {
          break; // Terminate based on counter value
        }
  
        std::cout << "[Child] Received: " << msg_recv.counter << std::endl;
  
        ipc::IPCMessage msg_send;
        if (msg_recv.counter < MAX_COUNT) {
  
          msg_send.counter = msg_recv.counter + 1;
          transport->send_message(msg_send);
          std::cout << "[Child] Sent: " << msg_send.counter << std::endl;
        } else {
  
          msg_send.counter = -1;
          transport->send_message(msg_send);
          std::cout << "[Child] Sent: termination" << std::endl;
        }
      }
      
    }

    exit(0);
  } else {

    // Parent process

    auto transport = IPCTransportFactory::create_transport(IPCType::MessageQueue);

    transport->initialize(QUEUE_BASE_NAME, false);
  
    ipc::IPCMessage msg_send;

    msg_send.counter = 0;

    if(transport->send_message(msg_send)) {

      std::cout << "[Parent] Sent initial: " << msg_send.counter << std::endl;

      ipc::IPCMessage msg_recv;
  
      while (true) {
  
        if(transport->receive_message(msg_recv)) {
  
          if (msg_recv.counter == -1) {
    
            break; // Terminate based on counter value
          }
    
          std::cout << "[Parent] Received: " << msg_recv.counter << std::endl;
    
          if (msg_recv.counter < MAX_COUNT) { // Adjust condition based on when to stop
    
            msg_send.counter = msg_recv.counter + 1;
            transport->send_message(msg_send);
            std::cout << "[Parent] Sent: " << msg_send.counter << std::endl;
          } else {
    
            msg_send.counter = -1;
            transport->send_message(msg_send);
            std::cout << "[Parent] Sent: termination" << std::endl;
          }
        }
      }
    }


    // wait(nullptr);
  }

  mq_unlink((QUEUE_BASE_NAME + "_ctp").c_str());
  mq_unlink((QUEUE_BASE_NAME + "_ptc").c_str());
}