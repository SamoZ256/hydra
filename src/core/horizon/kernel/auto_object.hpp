#pragma once

#include "core/horizon/kernel/const.hpp"

namespace hydra::horizon::kernel {

enum class AutoObjectTypeId {
    ClientPort,
    ClientSession,
    Port,
    ServerPort,
    ServerSession,
    Session,
    CodeMemory,
    Event, // TODO: ReadableEvent and WritableEvent
    Process,
    SharedMemory,
    Thread,
    TransferMemory,
};

class AutoObject {
  public:
    AutoObject(AutoObjectTypeId type_id_,
               const std::string_view debug_name_ = "AutoObject")
        : type_id{type_id_}, debug_name{
                                 fmt::format("{} {}", debug_name_,
                                             reinterpret_cast<void*>(this))} {}

    void Retain() { ref_count.fetch_add(1, std::memory_order_relaxed); }

    // Returns true if the object has been deallocated
    bool Release() {
        if (ref_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            // TODO: this assumes that the object is heap allocated, but that
            // may not always be the case
            delete this;
            return true;
        }

        return false;
    }

    template <typename T>
    bool IsOfType() const
        requires std::is_base_of_v<AutoObject, T>
    {
        return type_id == T::TYPE_ID;
    }

    std::string_view GetDebugName() const { return debug_name; }

  protected:
    virtual ~AutoObject() noexcept = default;

  private:
    AutoObjectTypeId type_id;
    std::string debug_name;

    std::atomic<u32> ref_count{1};

  public:
    GETTER(type_id, GetTypeId);
};

} // namespace hydra::horizon::kernel

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::AutoObjectTypeId, ClientPort,
                       "client port", ClientSession, "client session", Port,
                       "port", ServerPort, "server port", ServerSession,
                       "server session", Session, "session", CodeMemory,
                       "code memory", Event, "event", Process, "process",
                       SharedMemory, "shared memory", Thread, "thread",
                       TransferMemory, "transfer memory")
