#include <IPCTransportFactory.hpp>
#include <PipeTransport.hpp>

std::unique_ptr<ipc::IIPCTransport>
IPCTransportFactory::create_transport(IPCType type) {
  switch (type) {
  case IPCType::Pipe:
    return std::make_unique<ipc::PipeTransport>();
  case IPCType::SharedMemory:
    return std::make_unique<ipc::SharedMemoryTransport>();
  case IPCType::Socket:
    return std::make_unique<ipc::TCPSocketTransport>();
  case IPCType::MessageQueue:
    return std::make_unique<ipc::MsgQueueTransport>();
  case IPCType::Signal:
    return std::make_unique<ipc::SignalTransport>();
  }
  return std::unique_ptr<ipc::IIPCTransport>();
}
