#ifndef PIPE_TRANSPORT_HPP
#define PIPE_TRANSPORT_HPP

#include <IIPCTransport.hpp> // Include the base IPC transport interface
#include <unistd.h> // For POSIX pipe functions (e.g., open, close, read, write)

namespace ipc {

/*!
 * @brief Implements the IIPCTransport interface using named pipes (FIFOs).
 *
 * This class provides a concrete implementation for inter-process communication
 * using two named pipes to facilitate bidirectional message exchange between
 * processes. One pipe is used for sending messages, and the other for
 * receiving.
 */
class PipeTransport : public IIPCTransport {
public:
  /*!
   * @brief Constructs a new PipeTransport object.
   *
   * Initializes internal state variables. Named pipes are not created or opened
   * until the initialize method is called.
   */
  PipeTransport();

  /*!
   * @brief Destroys the PipeTransport object.
   *
   * Calls the cleanup method to ensure all open file descriptors are closed
   * and named pipes are unlinked if this instance was the creator.
   */
  ~PipeTransport() override;

  /*!
   * @brief Initializes the named pipe transport.
   *
   * This method creates or opens the two named pipes required for
   * communication. One process should call initialize with `create = true` to
   * create the pipes, and the other with `create = false` to connect to them.
   *
   * @param name A base name for the named pipes. Two pipes will be
   * created/opened (e.g., `name_pipe1` and `name_pipe2`).
   * @param create If true, attempts to create the named pipes. If false,
   * attempts to open existing named pipes.
   * @return True if initialization is successful, false otherwise.
   */
  bool initialize(const std::string &name, bool create) override;

  /*!
   * @brief Sends an IPCMessage through the named pipe.
   *
   * Writes the entire IPCMessage structure to the designated write pipe.
   *
   * @param msg A constant reference to the IPCMessage to be sent.
   * @return True if the message is successfully written, false otherwise.
   */
  bool send_message(const IPCMessage &msg) override;

  /*!
   * @brief Receives an IPCMessage from the named pipe.
   *
   * Reads an IPCMessage structure from the designated read pipe.
   *
   * @param msg A reference to an IPCMessage object where the received data will
   * be stored.
   * @return True if a message is successfully read, false otherwise.
   */
  bool receive_message(IPCMessage &msg) override;

  /*!
   * @brief Cleans up resources associated with the named pipe transport.
   *
   * Closes any open file descriptors and unlinks the named pipes if this
   * instance was responsible for their creation.
   */
  void cleanup() override;

private:
  /*! @brief The name of the first named pipe. */
  std::string pipe1_name;

  /*! @brief The name of the second named pipe. */
  std::string pipe2_name;

  /*! @brief File descriptor for reading from one of the pipes. Initialized to
   * -1. */
  int read_fd = -1;

  /*! @brief File descriptor for writing to the other pipe. Initialized to -1.
   */
  int write_fd = -1;

  /*!
   * @brief A flag indicating if this instance was the creator of the named
   * pipes.
   *
   * If true, this instance is responsible for unlinking the pipes during
   * cleanup.
   */
  bool is_creator = false;
};
} // namespace ipc

#endif // PIPE_TRANSPORT_HPP