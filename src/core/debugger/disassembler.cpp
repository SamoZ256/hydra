#include "core/debugger/disassembler.hpp"

#include "core/debugger/capstone/disassembler.hpp"

namespace hydra::debugger {

std::unique_ptr<IDisassembler> createDisassembler() {
    switch (CONFIG_INSTANCE.getDisassemblerBackend()) {
    case DisassemblerBackend::None:
        return nullptr;
    case DisassemblerBackend::Capstone:
        return capstone::Disassembler::create();
    default:
        // TODO: return an error instead
        LOG_FATAL(Other, "Unknown disassembler backend {}",
                  CONFIG_INSTANCE.getDisassemblerBackend());
    }
}

} // namespace hydra::debugger
