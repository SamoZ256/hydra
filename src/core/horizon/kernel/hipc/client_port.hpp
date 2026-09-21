#pragma once

#include "core/horizon/kernel/synchronization_object.hpp"

namespace hydra::horizon::kernel::hipc {

class Port;
class ClientSession;

class ClientPort : public SynchronizationObject {
  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::ClientPort;

    explicit ClientPort(std::string_view debug_name = "Client port")
        : SynchronizationObject(TYPE_ID, true, debug_name) {}

    ClientSession* connect();

  private:
    Port* parent;

  public:
    GETTER_AND_SETTER(parent, getParent, setParent);
};

} // namespace hydra::horizon::kernel::hipc
