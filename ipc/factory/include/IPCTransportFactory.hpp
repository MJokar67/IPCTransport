#ifndef IPC_TRANSPORT_FACTORY_HPP
#define IPC_TRANSPORT_FACTORY_HPP

#include <IIPCTransport.hpp> // Include the base IPC transport interface
#include <memory>            // For std::unique_ptr

/*!
 * @file IPC_TRANSPORT_FACTORY_HPP
 * @brief Defines a factory for creating various IPC transport mechanisms.
 *
 * This file provides an enumeration for different IPC types and a factory class
 * to create concrete implementations of the IIPCTransport interface based on
 * the specified type.
 */

/*!
 * @brief Enumerates the different types of Inter-Process Communication (IPC)
 * mechanisms.
 */
enum class IPCType {
  Pipe,         /*!< Represents a named pipe (FIFO) based IPC transport. */
  SharedMemory, /*!< Represents a shared memory based IPC transport. */
  Signal,       /*!< Represents a signal based IPC transport (e.g., for simple
                   notifications). */
  MessageQueue, /*!< Represents a message queue based IPC transport. */
  Socket        /*!< Represents a socket based IPC transport (e.g., Unix domain
                   sockets). */
};

/*!
 * @brief A factory class for creating instances of IIPCTransport.
 *
 * This class provides a static method to construct and return a unique pointer
 * to an IIPCTransport implementation based on the requested IPCType.
 * This decouples the client code from the concrete IPC transport classes.
 */
class IPCTransportFactory {
public:
  /*!
   * @brief Creates a unique pointer to an IIPCTransport implementation.
   *
   * This static method acts as a factory, instantiating the appropriate
   * IPC transport object based on the provided type.
   *
   * @param type The desired type of IPC transport to create (e.g., Pipe,
   * SharedMemory).
   * @return A std::unique_ptr to an IIPCTransport object if the type is
   * supported, or a nullptr if the type is unknown or creation fails.
   */
  static std::unique_ptr<ipc::IIPCTransport> create_transport(IPCType type);
};

#endif // IPC_TRANSPORT_FACTORY_HPP