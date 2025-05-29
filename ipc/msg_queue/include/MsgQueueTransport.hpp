#ifndef MSG_QUEUE_TRANSPORT_HPP
#define MSG_QUEUE_TRANSPORT_HPP

#include <IIPCTransport.hpp> // Include the base IPC transport interface
#include <cstring>           // For memcpy
#include <mqueue.h> // For POSIX message queue functions (mq_open, mq_send, mq_receive, mq_close, mq_unlink)
#include <sys/ipc.h> // For System V IPC key generation (ftok) - though POSIX mqueue is used, this might be a remnant or alternative consideration.
#include <sys/msg.h> // For System V message queue functions (msgget, msgsnd, msgrcv, msgctl) - this header seems to indicate System V, but mqd_t suggests POSIX. I will document based on mqd_t.
#include <sys/types.h> // For basic system data types
#include <sys/wait.h> // For waitpid (not directly used in this header, but often related to IPC processes)
#include <unistd.h>   // For POSIX functions

namespace ipc {

/*!
 * @brief Buffer structure for sending and receiving messages via message
 * queues.
 *
 * This structure is designed to hold the message type and the actual IPCMessage
 * data for transmission over a message queue. The `mtype` field is crucial
 * for message queue operations.
 */
struct MsgQueueBuffer {
  /*!
   * @brief Message type.
   *
   * Must be a positive integer. Used by message queue systems to filter
   * messages.
   */
  long mtype;

  /*!
   * @brief Buffer to hold the serialized IPCMessage data.
   *
   * Its size is exactly that of an IPCMessage to ensure proper data transfer.
   */
  char mtext[sizeof(IPCMessage)];
};

/*!
 * @brief Implements the IIPCTransport interface using POSIX message queues.
 *
 * This class provides a concrete implementation for inter-process communication
 * using system-wide message queues, allowing processes to send and receive
 * structured messages.
 */
class MsgQueueTransport : public IIPCTransport {
public:
  /*!
   * @brief Constructs a new MsgQueueTransport object.
   *
   * Initializes the message queue descriptor to an invalid state. The message
   * queue itself is opened or created during the `initialize` call.
   */
  MsgQueueTransport();

  /*!
   * @brief Destroys the MsgQueueTransport object.
   *
   * Calls the `cleanup` method to ensure the message queue is properly closed.
   */
  ~MsgQueueTransport();

  /*!
   * @brief Initializes the message queue transport.
   *
   * This method attempts to open an existing POSIX message queue or create a
   * new one.
   *
   * @param name A unique name for the message queue (e.g.,
   * "/my_message_queue"). This name must start with a slash '/'.
   * @param create A boolean flag. If true, attempts to create the message
   * queue; if false, attempts to open an existing one.
   * @return True if initialization is successful, false otherwise.
   */
  bool initialize(const std::string &name, bool create) override;

  /*!
   * @brief Sends an IPCMessage through the message queue.
   *
   * This method serializes the `IPCMessage` into a `MsgQueueBuffer` and sends
   * it to the message queue. A default message type (e.g., 1) is typically
   * used.
   *
   * @param msg A constant reference to the IPCMessage to be sent.
   * @return True if the message is successfully sent, false otherwise.
   */
  bool send_message(const IPCMessage &msg) override;

  /*!
   * @brief Receives an IPCMessage from the message queue.
   *
   * This method reads a message from the message queue into a `MsgQueueBuffer`
   * and deserializes it back into the provided `IPCMessage` structure.
   *
   * @param msg A reference to an IPCMessage object where the received data will
   * be stored.
   * @return True if a message is successfully received, false otherwise.
   */
  bool receive_message(IPCMessage &msg) override;

  /*!
   * @brief Cleans up resources associated with the message queue transport.
   *
   * Closes the message queue descriptor. If this instance was the creator,
   * it might also unlink (delete) the message queue from the system.
   */
  void cleanup() override;

private:
  /*! @brief The send message queue descriptor. Initialized to (mqd_t)-1, an invalid
   * descriptor. */
  mqd_t send_mq = (mqd_t)-1;
  /*! @brief The recieve message queue descriptor. Initialized to (mqd_t)-1, an invalid
   * descriptor. */
  mqd_t recieve_mq = (mqd_t)-1;
  /* @param send_name A unique name for the send message queue (e.g.,*/
  std::string send_name;
  /* @param recieve_name A unique name for the recieve message queue (e.g.,*/
  std::string recieve_name;
};
} // namespace ipc
#endif // MSG_QUEUE_TRANSPORT_HPP