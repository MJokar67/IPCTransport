#ifndef TCP_SOCKET_TRANSPORT_HPP
#define TCP_SOCKET_TRANSPORT_HPP

#include <IIPCTransport.hpp> // Include the base IPC transport interface
#include <netinet/in.h>      // For sockaddr_in, AF_INET, SOCK_STREAM, etc.
#include <string>            // For std::string
#include <unistd.h>          // For close()

namespace ipc {

/*!
 * @brief Implements the IIPCTransport interface using TCP sockets.
 *
 * This class provides a concrete implementation for inter-process communication
 * (or inter-machine communication) using TCP/IP sockets. It can operate in
 * either a server mode (listening for connections) or a client mode (connecting
 * to a server).
 */
class TCPSocketTransport : public IIPCTransport {
public:
  /*!
   * @brief Constructs a new TCPSocketTransport object.
   *
   * Initializes internal state variables to default values. Socket creation
   * and connection/listening occur during the `initialize` call.
   */
  TCPSocketTransport() = default;

  /*!
   * @brief Destroys the TCPSocketTransport object.
   *
   * Calls the `cleanup` method to ensure all open socket file descriptors
   * are properly closed.
   */
  ~TCPSocketTransport();

  /*!
   * @brief Initializes the TCP socket transport.
   *
   * If `create` is true, this instance acts as a server, binding to the
   * specified port and listening for incoming connections. If `create` is
   * false, this instance acts as a client, attempting to connect to a server at
   * the specified address and port.
   *
   * @param name A string representing the IP address (for client) or a
   * placeholder (for server, typically "0.0.0.0" or "127.0.0.1" for local). The
   * port number is expected to be part of this string or implicitly handled by
   * the implementation details. (Note: A more robust design might separate
   * address and port).
   * @param create A boolean flag. If true, this instance acts as a server;
   * if false, it acts as a client.
   * @return True if initialization (binding/listening or connecting) is
   * successful, false otherwise.
   */
  bool initialize(const std::string &name, bool create) override;

  /*!
   * @brief Sends an IPCMessage through the TCP socket.
   *
   * This method serializes the `IPCMessage` and sends its raw bytes over the
   * established TCP connection. It ensures all bytes are sent.
   *
   * @param msg A constant reference to the IPCMessage to be sent.
   * @return True if the message is successfully sent, false otherwise.
   */
  bool send_message(const IPCMessage &msg) override;

  /*!
   * @brief Receives an IPCMessage from the TCP socket.
   *
   * This method reads the raw bytes of an `IPCMessage` from the TCP connection
   * and deserializes them into the provided `IPCMessage` structure. It ensures
   * all expected bytes are received.
   *
   * @param msg A reference to an IPCMessage object where the received data will
   * be stored.
   * @return True if a message is successfully received, false otherwise.
   */
  bool receive_message(IPCMessage &msg) override;

  /*!
   * @brief Cleans up resources associated with the TCP socket transport.
   *
   * Closes all open socket file descriptors (listening socket, client
   * connection socket).
   */
  void cleanup() override;

private:
  /*! @brief The main socket file descriptor (listening socket for server,
   * connecting socket for client). */
  int socket_fd = -1;

  /*! @brief The client connection socket file descriptor (used only by the
   * server after accepting a connection). */
  int client_fd = -1; // used only in server

  /*! @brief Structure holding socket address information (IP address and port).
   */
  sockaddr_in addr{};

  /*! @brief A flag indicating if this instance is operating as a server. */
  bool is_server = false;

  /*!
   * @brief Helper function to ensure all bytes from a buffer are sent over a
   * socket.
   *
   * This function repeatedly calls `send()` until the entire `length` of the
   * `buffer` has been sent, handling partial sends.
   *
   * @param fd The socket file descriptor to send data through.
   * @param buffer A pointer to the character array containing the data to send.
   * @param length The total number of bytes to send.
   * @return True if all bytes were sent successfully, false otherwise (e.g.,
   * connection closed).
   */
  bool send_all(int fd, const char *buffer, size_t length);

  /*!
   * @brief Helper function to ensure all expected bytes are received from a
   * socket.
   *
   * This function repeatedly calls `recv()` until the entire `length` of data
   * has been received into the `buffer`, handling partial receives.
   *
   * @param fd The socket file descriptor to receive data from.
   * @param buffer A pointer to the character array where received data will be
   * stored.
   * @param length The total number of bytes expected to receive.
   * @return True if all bytes were received successfully, false otherwise
   * (e.g., connection closed).
   */
  bool recv_all(int fd, char *buffer, size_t length);
};
} // namespace ipc

#endif // TCP_SOCKET_TRANSPORT_HPP