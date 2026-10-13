#include "core/debugger/disassembler.hpp"

#include "core/debugger/capstone/disassembler.hpp"

namespace hydra::debugger {

std::unique_ptr<IDisassembler> createDisassembler() {
    return capstone::Disassembler::create();
}

} // namespace hydra::debugger
