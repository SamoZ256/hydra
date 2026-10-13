#pragma once

#include "core/debugger/disassembler.hpp"
#include "core/debugger/symbol_table.hpp"

namespace hydra::debugger {

constexpr u32 DEFAULT_LISTING_INSTRUCTIONS = 32;
constexpr u32 MAX_LISTING_INSTRUCTIONS = 0x4000;

enum class DisassemblyError {
    NoTarget,
    Unavailable,
    Unaligned,
    Unmapped,
    NoFunction,
};

struct DisassemblyRequest {
    vaddr_t addr;
    u32 count;
    std::optional<vaddr_t> pc{std::nullopt};
};

struct ListingLine {
    Instruction instruction;
    std::optional<std::string> label{std::nullopt};
    std::optional<SymbolMatch> target_symbol{std::nullopt};
    std::optional<SymbolMatch> data_symbol{std::nullopt};
    bool has_breakpoint{false};
    bool is_pc{false};
};

struct Listing {
    vaddr_t addr;
    std::optional<SymbolMatch> location{std::nullopt};
    std::vector<ListingLine> lines;
    bool truncated{false};
};

struct ListingContext {
    const SymbolTable& module_table;
    const SymbolTable& function_table;
    std::span<const vaddr_t> breakpoints;
    std::optional<vaddr_t> pc;
};

Listing buildListing(vaddr_t addr, std::vector<Instruction> instructions,
                     const ListingContext& context);
std::string formatSymbol(const SymbolMatch& match);
std::string formatListing(const Listing& listing);

} // namespace hydra::debugger

ENABLE_ENUM_FORMATTING(hydra::debugger::DisassemblyError, NoTarget, "no target",
                       Unavailable, "disassembler unavailable", Unaligned,
                       "address is not 4-byte aligned", Unmapped,
                       "address is not mapped", NoFunction,
                       "no function symbol covers the address")
