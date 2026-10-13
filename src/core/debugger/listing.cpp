#include "core/debugger/listing.hpp"

#include <algorithm>

namespace hydra::debugger {

namespace {

std::optional<SymbolMatch> resolveSymbol(const ListingContext& context,
                                         vaddr_t addr) {
    if (auto match = context.function_table.tryResolve(addr))
        return match;

    return context.module_table.tryResolve(addr);
}

std::optional<std::string> findLabel(const ListingContext& context,
                                     vaddr_t addr) {
    if (auto label = context.function_table.tryFindStartingAt(addr))
        return label;

    return context.module_table.tryFindStartingAt(addr);
}

} // namespace

std::string formatSymbol(const SymbolMatch& match) {
    if (match.offset == 0)
        return match.name;

    return fmt::format("{}+{:#x}", match.name, match.offset);
}

Listing buildListing(vaddr_t addr, std::vector<Instruction> instructions,
                     const ListingContext& context) {
    Listing listing{.addr = addr, .location = resolveSymbol(context, addr)};
    listing.lines.reserve(instructions.size());
    for (auto& instruction : instructions) {
        ListingLine line{.instruction = std::move(instruction)};
        const auto& insn = line.instruction;
        line.label = findLabel(context, insn.addr);
        if (insn.target)
            line.target_symbol = resolveSymbol(context, *insn.target);
        if (insn.data_ref)
            line.data_symbol = resolveSymbol(context, *insn.data_ref);
        line.has_breakpoint =
            std::ranges::binary_search(context.breakpoints, insn.addr);
        line.is_pc = context.pc == insn.addr;
        listing.lines.push_back(std::move(line));
    }

    return listing;
}

std::string formatListing(const Listing& listing) {
    std::string text;
    if (listing.location &&
        (listing.lines.empty() || !listing.lines.front().label))
        text += fmt::format("<{}>:\n", formatSymbol(*listing.location));

    for (const auto& line : listing.lines) {
        if (line.label)
            text += fmt::format("{}:\n", *line.label);

        const auto& insn = line.instruction;
        text +=
            fmt::format("{}{} {:016x}  {:08x}  ", line.is_pc ? "=>" : "  ",
                        line.has_breakpoint ? '*' : ' ', insn.addr, insn.word);
        if (insn.operands.empty())
            text += insn.mnemonic;
        else
            text += fmt::format("{:<8} {}", insn.mnemonic, insn.operands);

        const auto& symbol =
            line.target_symbol ? line.target_symbol : line.data_symbol;
        if (symbol)
            text += fmt::format(" <{}>", formatSymbol(*symbol));
        text += '\n';
    }

    return text;
}

} // namespace hydra::debugger
