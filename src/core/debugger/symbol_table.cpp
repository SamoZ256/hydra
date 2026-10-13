#include "core/debugger/symbol_table.hpp"

#include <algorithm>
#include <functional>

namespace hydra::debugger {

namespace {

vaddr_t symbolBegin(const Symbol& symbol) {
    return symbol.guest_mem_range.getBegin();
}

bool beginsBefore(const Symbol& a, const Symbol& b) {
    return symbolBegin(a) < symbolBegin(b);
}

} // namespace

void SymbolTable::registerSymbol(const Symbol& symbol) {
    std::scoped_lock lock(mutex);

    const auto it = std::ranges::upper_bound(symbols, symbol, beginsBefore);
    const auto index = static_cast<usize>(std::distance(symbols.begin(), it));
    symbols.insert(it, symbol);
    rebuildMaxEndsLocked(index);
}

void SymbolTable::registerSymbols(std::span<const Symbol> new_symbols) {
    std::scoped_lock lock(mutex);

    symbols.insert(symbols.end(), new_symbols.begin(), new_symbols.end());
    std::ranges::stable_sort(symbols, beginsBefore);
    rebuildMaxEndsLocked(0);
}

std::string SymbolTable::findSymbol(vaddr_t addr) const {
    std::scoped_lock lock(mutex);

    const auto index = findIndexLocked(addr);
    return index ? symbols[*index].name : "";
}

std::optional<SymbolMatch> SymbolTable::tryResolve(vaddr_t addr) const {
    std::scoped_lock lock(mutex);

    const auto index = findIndexLocked(addr);
    if (!index)
        return std::nullopt;

    const auto& symbol = symbols[*index];
    return SymbolMatch{.name = symbol.name,
                       .range = symbol.guest_mem_range,
                       .offset = addr - symbol.guest_mem_range.getBegin()};
}

std::optional<std::string> SymbolTable::tryFindStartingAt(vaddr_t addr) const {
    std::scoped_lock lock(mutex);

    const auto it = std::ranges::lower_bound(symbols, addr, std::ranges::less{},
                                             symbolBegin);
    if (it == symbols.end() || symbolBegin(*it) != addr)
        return std::nullopt;

    return it->name;
}

std::vector<Symbol> SymbolTable::getSymbols() const {
    std::scoped_lock lock(mutex);
    return symbols;
}

std::optional<usize> SymbolTable::findIndexLocked(vaddr_t addr) const {
    const auto it = std::ranges::upper_bound(symbols, addr, std::ranges::less{},
                                             symbolBegin);
    auto i = static_cast<usize>(std::distance(symbols.begin(), it));
    while (i > 0) {
        i--;
        if (max_ends[i] <= addr)
            return std::nullopt;

        if (!symbols[i].guest_mem_range.contains(addr))
            continue;

        const auto begin = symbolBegin(symbols[i]);
        while (i > 0 && symbolBegin(symbols[i - 1]) == begin &&
               symbols[i - 1].guest_mem_range.contains(addr))
            i--;

        return i;
    }

    return std::nullopt;
}

void SymbolTable::rebuildMaxEndsLocked(usize from) {
    max_ends.resize(symbols.size());
    vaddr_t max_end = from > 0 ? max_ends[from - 1] : 0;
    for (usize i = from; i < symbols.size(); i++) {
        max_end = std::max(max_end, symbols[i].guest_mem_range.getEnd());
        max_ends[i] = max_end;
    }
}

} // namespace hydra::debugger
