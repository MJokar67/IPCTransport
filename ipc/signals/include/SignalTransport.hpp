#ifndef SIGNAL_TRANSPORT_HPP
#define SIGNAL_TRANSPORT_HPP

#include <IIPCTransport.hpp> // Include the base IPC transport interface
#include <atomic>            // For std::atomic<bool>
#include <cerrno>            // For errno
#include <csignal> // For signal handling (sigaction, kill, sigemptyset, sigaddset, sigprocmask)
#include <cstring>    // For strerror
#include <fcntl.h>    // For file control options (O_CREAT, O_RDWR)
#include <sys/mman.h> // For shared memory (shm_open, mmap, munmap, shm_unlink)
#include <unistd.h>   // For POSIX functions (ftruncate, close, getpid)

namespace ipc {

/*!
 * @brief Implements the IIPCTransport interface using POSIX signals and shared
 * memory.
 *
 * This class provides an IPC mechanism where signals (specifically SIGUSR1) are
 * used to notify a peer process about the availability of new data, while the
 * actual data is exchanged via a shared memory segment. This combines the
 * notification efficiency of signals with the data transfer capability of
 * shared memory.
 */
class SignalTransport : public IIPCTransport {
public:
  /*!
   * @brief Constructs a new SignalTransport object.
   *
   * Initializes internal state variables. Shared memory and signal handlers are
   * set up during the `initialize` method call.
   */
  SignalTransport();

  /*!
   * @brief Destroys the SignalTransport object.
   *
   * Calls the `cleanup` method to ensure the shared memory segment is unmapped,
   * file descriptor is closed, and the shared memory object is unlinked if
   * this instance was the owner. It also restores the default signal handler.
   */
  ~SignalTransport() override;

  /*!
   * @brief Initializes the signal and shared memory transport.
   *
   * This method creates or opens a POSIX shared memory object, maps it into
   * the process's address space, and sets up a signal handler for SIGUSR1.
   * One process should call initialize with `create = true` to create the SHM,
   * and the other with `create = false` to connect to it.
   *
   * @param name A unique name for the shared memory object (e.g.,
   * "/my_signal_shm").
   * @param create A boolean flag indicating whether to create the shared memory
   * object if it doesn't exist (true), or to connect to an existing one
   * (false).
   * @return True if initialization is successful, false otherwise.
   */
  bool initialize(const std::string &name, bool create) override;

  /*!
   * @brief Sends an IPCMessage through the shared memory segment and signals
   * the peer.
   *
   * This method copies the provided message into the shared memory segment and
   * then sends a `SIGUSR1` signal to the peer process to notify it of new data.
   *
   * @param msg A constant reference to the IPCMessage to be sent.
   * @return True if the message is successfully written and the signal is sent,
   * false otherwise.
   */
  bool send_message(const IPCMessage &msg) override;

  /*!
   * @brief Receives an IPCMessage from the shared memory segment, waiting for a
   * signal.
   *
   * This method waits for a `SIGUSR1` signal from the peer process, indicating
   * that new data is available in the shared memory. Once the signal is
   * received, it copies the message from shared memory into the provided
   * structure.
   *
   * @param msg A reference to an IPCMessage object where the received data will
   * be stored.
   * @return True if a message is successfully received, false otherwise.
   */
  bool receive_message(IPCMessage &msg) override;

  /*!
   * @brief Cleans up resources associated with the signal and shared memory
   * transport.
   *
   * Unmaps the shared memory segment, closes the shared memory file descriptor,
   * and unlinks the shared memory object from the file system if this instance
   * was the owner. It also restores the default signal handler for SIGUSR1.
   */
  void cleanup() override;

  /*!
   * @brief Sets the Process ID (PID) of the peer process.
   *
   * This PID is used by the `send_message` method to send signals to the
   * correct process. This method must be called by both processes after they
   * know each other's PIDs.
   *
   * @param pid The process ID of the peer.
   */
  void setPeerPid(pid_t pid);

private:
  /*! @brief The size of the shared memory segment, equal to the size of
   * IPCMessage. */
  static constexpr size_t SHM_SIZE = sizeof(IPCMessage);

  /*! @brief The name of the POSIX shared memory object. */
  std::string shm_name;

  /*! @brief File descriptor for the shared memory object. Initialized to -1. */
  int shm_fd = -1;

  /*!
   * @brief Pointer to the mapped IPCMessage structure in shared memory.
   *
   * This pointer provides direct access to the shared data.
   */
  IPCMessage *shared_msg = nullptr;

  /*!
   * @brief The Process ID (PID) of the peer process.
   *
   * Used to send signals to the other process. Initialized to -1.
   */
  pid_t peer_pid = -1;

  /*!
   * @brief An atomic flag indicating whether a signal has been received.
   *
   * This static member is used by the signal handler to communicate signal
   * reception to the main thread, ensuring thread-safe updates.
   */
  static std::atomic<bool> signal_received;

  /*!
   * @brief Static signal handler for SIGUSR1.
   *
   * This function is called by the operating system when SIGUSR1 is received.
   * It sets the `signal_received` atomic flag to true.
   * @param signum The signal number that triggered the handler (expected to be
   * SIGUSR1).
   */
  static void signal_handler(int signum);

  /*!
   * @brief Waits for the `signal_received` flag to be set.
   *
   * This private helper function blocks until the `signal_received` atomic flag
   * becomes true, indicating that a signal has been caught by the
   * `signal_handler`. It then resets the flag.
   * @return True if a signal was successfully waited for and processed.
   */
  bool wait_for_signal();
};
} // namespace ipc

#endif // SIGNAL_TRANSPORT_HPP