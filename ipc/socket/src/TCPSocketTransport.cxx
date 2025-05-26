#include <TCPSocketTransport.hpp>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <unistd.h>

ipc::TCPSocketTransport::~TCPSocketTransport() { cleanup(); }

bool ipc::TCPSocketTransport::initialize(const std::string &name, bool create) {
  // name format: "ip:port", e.g. "127.0.0.1:12345"
  size_t colon_pos = name.find(':');
  if (colon_pos == std::string::npos) {
    std::cerr << "Invalid address format, expected ip:port\n";
    return false;
  }
  std::string ip = name.substr(0, colon_pos);
  int port = std::stoi(name.substr(colon_pos + 1));

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) <= 0) {
    std::cerr << "Invalid IP address\n";
    return false;
  }

  if (create) {
    // Server
    is_server = true;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
      perror("socket");
      return false;
    }

    int opt = 1;
    setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(socket_fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
      perror("bind");
      cleanup();
      return false;
    }

    if (listen(socket_fd, 1) < 0) {
      perror("listen");
      cleanup();
      return false;
    }

    std::cout << "[Server] Listening on " << ip << ":" << port << "\n";
    client_fd = accept(socket_fd, nullptr, nullptr);
    if (client_fd < 0) {
      perror("accept");
      cleanup();
      return false;
    }

    std::cout << "[Server] Client connected\n";

  } else {
    // Client
    is_server = false;
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
      perror("socket");
      return false;
    }

    if (connect(socket_fd, (sockaddr *)&addr, sizeof(addr)) < 0) {
      perror("connect");
      cleanup();
      return false;
    }

    std::cout << "[Client] Connected to server\n";
  }

  return true;
}

bool ipc::TCPSocketTransport::send_message(const IPCMessage &msg) {
  int fd = is_server ? client_fd : socket_fd;
  return send_all(fd, reinterpret_cast<const char *>(&msg), sizeof(IPCMessage));
}

bool ipc::TCPSocketTransport::receive_message(IPCMessage &msg) {
  int fd = is_server ? client_fd : socket_fd;
  return recv_all(fd, reinterpret_cast<char *>(&msg), sizeof(IPCMessage));
}

void ipc::TCPSocketTransport::cleanup() {
  if (client_fd != -1) {
    close(client_fd);
    client_fd = -1;
  }
  if (socket_fd != -1) {
    close(socket_fd);
    socket_fd = -1;
  }
}

bool ipc::TCPSocketTransport::send_all(int fd, const char *buffer,
                                       size_t length) {
  size_t total_sent = 0;
  while (total_sent < length) {
    ssize_t sent = send(fd, buffer + total_sent, length - total_sent, 0);
    if (sent <= 0) {
      if (sent < 0 && errno == EINTR)
        continue; // interrupted, retry
      perror("send");
      return false;
    }
    total_sent += sent;
  }
  return true;
}

bool ipc::TCPSocketTransport::recv_all(int fd, char *buffer, size_t length) {
  size_t total_received = 0;
  while (total_received < length) {
    ssize_t recvd =
        recv(fd, buffer + total_received, length - total_received, 0);
    if (recvd <= 0) {
      if (recvd < 0 && errno == EINTR)
        continue; // interrupted, retry
      perror("recv");
      return false;
    }
    total_received += recvd;
  }
  return true;
}
