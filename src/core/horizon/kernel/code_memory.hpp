#pragma once

#include "core/horizon/kernel/auto_object.hpp"

namespace hydra::horizon::kernel {

// TODO: does this inherit from AutoObject?
class CodeMemory : public AutoObject {
  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::CodeMemory;

    CodeMemory(vaddr_t addr_, u64 size_,
               const std::string_view debug_name = "CodeMemory")
        : AutoObject(TYPE_ID, debug_name), addr{addr_}, size{size_} {}

  private:
    vaddr_t addr;
    u64 size;

  public:
    GETTER(addr, GetAddress);
    GETTER(size, GetSize);
};

} // namespace hydra::horizon::kernel
