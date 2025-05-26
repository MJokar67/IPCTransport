#ifndef IPC_MESSAGE_HPP
#define IPC_MESSAGE_HPP

#include <cstdint>
#include <pthread.h>

namespace ipc {

/*!
 * @brief Represents a message structure for inter-process communication.
 *
 * This structure contains a counter, status flags (ready and finished),
 * and a data buffer to facilitate communication between processes.
 */
struct IPCMessage {
  /*! @brief A counter for tracking the message or its sequence. */
  uint32_t counter;

  /*! @brief A flag indicating whether the message is ready for processing. */
  bool ready;

  /*! @brief A flag indicating whether the processing of the message is
   * finished. */
  bool finished;

  /*! @brief A data buffer of 256 characters to hold the message content.
   *
   * Initialized to all zeros.
   */
  char data[256] = {0};

  /*! @brief Default constructor for the IPCMessage structure. */
  IPCMessage() = default;
};
} // namespace ipc

#endif // IPC_MESSAGE_HPP