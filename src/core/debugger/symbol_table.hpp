#pragma once

#include <mutex>

namespace hydra::debugger {

struct Symbol {
    std::string name;
    ztd::Range<vaddr_t> guest_mem_range;
};

struct SymbolMatch {
    std::string name;
    ztd::Range<vaddr_t> range;
    u64 offset;
};

class SymbolTable {
  public:
    void registerSymbol(const Symbol& symbol);
    void registerSymbols(std::span<const Symbol> new_symbols);

    std::string findSymbol(vaddr_t addr) const;
    std::optional<SymbolMatch> tryResolve(vaddr_t addr) const;
    std::optional<std::string> tryFindStartingAt(vaddr_t addr) const;
    std::vector<Symbol> getSymbols() const;

  private:
    mutable std::mutex mutex;
    std::vector<Symbol> symbols;
    std::vector<vaddr_t> max_ends;

    std::optional<usize> findIndexLocked(vaddr_t addr) const;
    void rebuildMaxEndsLocked(usize from);
};

} // namespace hydra::debugger
