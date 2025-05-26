#include <PipeTransport.hpp>
#include <fcntl.h>
#include <sys/stat.h>

ipc::PipeTransport::PipeTransport() = default;

ipc::PipeTransport::~PipeTransport() { cleanup(); }

bool ipc::PipeTransport::initialize(const std::string &name, bool create) {
  is_creator = create;
  pipe1_name = "/tmp/" + name + "_pipe1";
  pipe2_name = "/tmp/" + name + "_pipe2";

  if (create) {
    // Create the FIFOs
    mkfifo(pipe1_name.c_str(), 0666);
    mkfifo(pipe2_name.c_str(), 0666);

    // Parent writes to pipe1, reads from pipe2
    write_fd = open(pipe1_name.c_str(), O_WRONLY);
    read_fd = open(pipe2_name.c_str(), O_RDONLY);
  } else {
    // Child reads from pipe1, writes to pipe2
    read_fd = open(pipe1_name.c_str(), O_RDONLY);
    write_fd = open(pipe2_name.c_str(), O_WRONLY);
  }

  return (read_fd != -1 && write_fd != -1);
}

bool ipc::PipeTransport::send_message(const IPCMessage &msg) {
  ssize_t written = write(write_fd, &msg, sizeof(msg));
  return written == sizeof(msg);
}

bool ipc::PipeTransport::receive_message(IPCMessage &msg) {
  ssize_t read_bytes = read(read_fd, &msg, sizeof(msg));
  return read_bytes == sizeof(msg);
}

void ipc::PipeTransport::cleanup() {
  if (read_fd != -1) {
    close(read_fd);
    read_fd = -1;
  }
  if (write_fd != -1) {
    close(write_fd);
    write_fd = -1;
  }
  if (is_creator) {
    unlink(pipe1_name.c_str());
    unlink(pipe2_name.c_str());
  }
}
