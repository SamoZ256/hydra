#pragma once

#include "core/horizon/kernel/hipc/client_port.hpp"

namespace hydra::horizon::kernel::hipc {

class ClientPort;

template <typename Key>
class ServiceManager {
  public:
    ~ServiceManager() {
        std::scoped_lock lock(mutex);
        for (auto& [_, port] : ports)
            port->release();
    }

    void registerPort(const Key& port_name, ClientPort* client_port) {
        std::scoped_lock lock(mutex);
        client_port->retain();
        ports.insert({port_name, client_port});
    }

    void unregisterPort(const Key& port_name) {
        std::scoped_lock lock(mutex);
        auto it = ports.find(port_name);
        ASSERT(it != ports.end(), Kernel, "Port not registered");
        it->second->Release();
        ports.erase(it);
    }

    ClientPort* getPort(const Key& port_name) {
        std::scoped_lock lock(mutex);
        auto it = ports.find(port_name);
        if (it == ports.end())
            return nullptr;

        return it->second;
    }

  private:
    std::mutex mutex;
    std::map<Key, ClientPort*> ports;
};

} // namespace hydra::horizon::kernel::hipc
