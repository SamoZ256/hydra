#pragma once

#include "core/horizon/kernel/synchronization_object.hpp"

namespace hydra::horizon::kernel {

// TODO: ReadableEvent and WritableEvent
class Event : public SynchronizationObject {
  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::Event;

    Event(bool signalled = false, const std::string_view debug_name = "Event")
        : SynchronizationObject(TYPE_ID, signalled, debug_name) {}
};

} // namespace hydra::horizon::kernel
