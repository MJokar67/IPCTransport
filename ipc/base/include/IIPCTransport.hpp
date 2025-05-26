#ifndef IPS_TRANSPORT_HPP
#define IPS_TRANSPORT_HPP

#include "IIPCMessage.hpp"
#include <string>

namespace ipc {

/*!
 * @brief An abstract interface for inter-process communication (IPC) transport mechanisms.
 *
 * This interface defines the core functionalities required for sending and receiving
 * IPC messages, allowing for different underlying IPC implementations (e.g., shared memory,
 * message queues, sockets) to adhere to a common API.
 */
class IIPCTransport {
public:
  /*!
   * @brief Virtual destructor for the IIPCTransport interface.
   *
   * Ensures proper cleanup of derived class resources when an IIPCTransport pointer is deleted.
   */
  virtual ~IIPCTransport() = default;

  /*!
   * @brief Initializes the IPC transport mechanism.
   *
   * This method sets up the necessary resources for communication.
   *
   * @param name A unique identifier or name for the IPC resource (e.g., shared memory key, queue name).
   * @param create A boolean flag indicating whether to create the IPC resource if it doesn't exist (true),
   * or to connect to an existing one (false).
   * @return True if initialization is successful, false otherwise.
   */
  virtual bool initialize(const std::string &name, bool create) = 0;

  /*!
   * @brief Sends an IPC message through the transport.
   *
   * This method writes the provided message to the IPC channel.
   *
   * @param msg A constant reference to the IPCMessage to be sent.
   * @return True if the message is sent successfully, false otherwise.
   */
  virtual bool send_message(const IPCMessage &msg) = 0;

  /*!
   * @brief Receives an IPC message from the transport.
   *
   * This method reads a message from the IPC channel into the provided message structure.
   *
   * @param msg A reference to an IPCMessage object where the received data will be stored.
   * @return True if a message is received successfully, false otherwise.
   */
  virtual bool receive_message(IPCMessage &msg) = 0;

  /*!
   * @brief Cleans up any resources allocated by the IPC transport.
   *
   * This method should be called to release system resources associated with the IPC mechanism.
   */
  virtual void cleanup() = 0;
};
} // namespace ipc

#endif // IPS_TRANSPORT_HPP