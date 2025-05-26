// ipc/SharedMemoryTransport.hpp

#ifndef IPS_TRANSPORT_SHM_HPP
#define IPS_TRANSPORT_SHM_HPP

#include <IIPCTransport.hpp> // Include the base IPC transport interface
#include <fcntl.h>           // For file control options (e.g., O_CREAT, O_RDWR)
#include <pthread.h>         // For POSIX threads mutex and condition variables
#include <string>            // For std::string
#include <sys/mman.h> // For memory mapping functions (e.g., shm_open, mmap, munmap, shm_unlink)
#include <unistd.h> // For POSIX functions (e.g., ftruncate, close)

namespace ipc {

/*!
 * @brief Extends IPCMessage with synchronization primitives for shared memory.
 *
 * This structure is designed to be placed directly into shared memory.
 * It includes a mutex and a condition variable to facilitate synchronized
 * access and signaling between processes sharing this message.
 */
struct IPCMessageSHM : IPCMessage {
  /*!
   * @brief Mutex for protecting access to the shared message data.
   *
   * Aligned to 64 bytes to potentially improve performance on some
   * architectures. This mutex should be initialized for process-shared use.
   */
  alignas(64) pthread_mutex_t mutex;

  /*!
   * @brief Condition variable for signaling changes to the shared message data.
   *
   * This condition variable should be initialized for process-shared use.
   */
  pthread_cond_t cond;

  /*! @brief Default constructor for IPCMessageSHM. */
  IPCMessageSHM() = default;
};

/*!
 * @brief Implements the IIPCTransport interface using POSIX shared memory.
 *
 * This class provides a concrete IPC transport mechanism that uses a shared
 * memory segment for message exchange. It incorporates a mutex and condition
 * variable within the shared memory for robust synchronization between
 * processes.
 */
class SharedMemoryTransport : public IIPCTransport {
public:
  /*!
   * @brief Constructs a new SharedMemoryTransport object.
   *
   * Initializes internal state variables. Shared memory segment is not created
   * or mapped until the initialize method is called.
   */
  SharedMemoryTransport();

  /*!
   * @brief Destroys the SharedMemoryTransport object.
   *
   * Calls the cleanup method to ensure the shared memory segment is unmapped,
   * file descriptor is closed, and the shared memory object is unlinked if
   * this instance was the owner.
   */
  ~SharedMemoryTransport() override;

  /*!
   * @brief Initializes the shared memory transport.
   *
   * This method creates or opens a POSIX shared memory object, maps it into
   * the process's address space, and initializes the synchronization primitives
   * (mutex and condition variable) for process-shared use.
   *
   * @param name A unique name for the shared memory object (e.g.,
   * "/my_shm_object").
   * @param create A boolean flag indicating whether to create the shared memory
   * object if it doesn't exist (true), or to connect to an existing one
   * (false).
   * @return True if initialization is successful, false otherwise.
   */
  bool initialize(const std::string &name, bool create) override;

  /*!
   * @brief Sends an IPCMessage through the shared memory segment.
   *
   * This method copies the provided message into the shared memory segment
   * and uses the shared mutex and condition variable to signal the receiver.
   * It ensures thread-safe access to the shared message.
   *
   * @param msg A constant reference to the IPCMessage to be sent.
   * @return True if the message is successfully written and signaled, false
   * otherwise.
   */
  bool send_message(const IPCMessage &msg) override;

  /*!
   * @brief Receives an IPCMessage from the shared memory segment.
   *
   * This method reads a message from the shared memory segment, using the
   * shared mutex and condition variable to wait for new messages and ensure
   * thread-safe access.
   *
   * @param msg A reference to an IPCMessage object where the received data will
   * be stored.
   * @return True if a message is successfully read, false otherwise.
   */
  bool receive_message(IPCMessage &msg) override;

  /*!
   * @brief Cleans up resources associated with the shared memory transport.
   *
   * Unmaps the shared memory segment, closes the shared memory file descriptor,
   * and unlinks the shared memory object from the file system if this instance
   * was the owner. It also destroys the process-shared mutex and condition
   * variable.
   */
  void cleanup() override;

  /*!
   * @brief Retrieves a pointer to the shared IPCMessageSHM object.
   *
   * This method provides direct access to the mapped shared memory message
   * for advanced use cases, but care must be taken to ensure proper
   * synchronization.
   *
   * @return A pointer to the IPCMessageSHM object in shared memory, or nullptr
   * if the shared memory has not been successfully initialized.
   */
  IPCMessage *get_shared_message() const;

private:
  /*! @brief The name of the POSIX shared memory object. */
  std::string shm_name;

  /*! @brief File descriptor for the shared memory object. Initialized to -1. */
  int shm_fd = -1;

  /*!
   * @brief Pointer to the mapped IPCMessageSHM structure in shared memory.
   *
   * This pointer provides direct access to the shared data and synchronization
   * primitives.
   */
  IPCMessageSHM *shared_msg = nullptr;

  /*!
   * @brief A flag indicating if this instance is the owner of the shared memory
   * object.
   *
   * If true, this instance is responsible for unlinking the shared memory
   * object during cleanup.
   */
  bool is_owner = false;
};
} // namespace ipc

#endif // IPS_TRANSPORT_SHM_HPP