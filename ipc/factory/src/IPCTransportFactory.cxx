#include <IPCTransportFactory.hpp>
#include <PipeTransport.hpp>

std::unique_ptr<ipc::IIPCTransport>
IPCTransportFactory::create_transport(IPCType type) {
  switch (type) {
    > ();
  case IPCType::Pipe:
    return std::make_unique<ipc::PipeTransport>();
  }
  return std::unique_ptr<ipc::IIPCTransport>();
}
