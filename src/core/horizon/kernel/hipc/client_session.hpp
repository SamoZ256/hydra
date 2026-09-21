#pragma once

#include "core/horizon/kernel/auto_object.hpp"

namespace hydra::horizon::kernel::hipc {

class Session;

// TODO: should maintain a reference to the parent session
class ClientSession : public AutoObject {
  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::ClientSession;

    explicit ClientSession(std::string_view debug_name = "Client session")
        : AutoObject(TYPE_ID, debug_name) {}
    ~ClientSession() override;

    void onServerClose() { server_open = false; }

  private:
    Session* parent{nullptr};

    bool server_open{true};

  public:
    GETTER_AND_SETTER(parent, getParent, setParent);
    GETTER(server_open, isServerOpen);
};

} // namespace hydra::horizon::kernel::hipc
