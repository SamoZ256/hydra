#include "core/horizon/loader/pfs0_loader.hpp"

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/filesystem/host_file.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/loader/nso_loader.hpp"

namespace hydra::horizon::loader {

namespace {

struct Pfs0Header {
    u32 magic;
    u32 entry_count;
    u32 string_table_size;
    u32 reserved;
};

} // namespace

Pfs0Loader::Pfs0Loader(StreamReader reader) {
    // Header
    const auto header = reader.Read<Pfs0Header>();
    ASSERT(header.magic == make_magic4('P', 'F', 'S', '0'), Loader,
           "Invalid PFS0 magic");

    // Entries
    entries.resize(header.entry_count);
    reader.ReadPtr(entries.data(), header.entry_count);

    // String table
    string_table.resize(header.string_table_size);
    reader.ReadPtr(string_table.data(), header.string_table_size);

    entries_offset = reader.Tell();
}

kernel::Process* Pfs0Loader::LoadProcess(StreamReader reader,
                                         const std::string_view rom_filename) {
    kernel::Process* process = nullptr;

    // NSOs
    for (const auto& entry : entries) {
        const std::string entry_name(string_table.data() + entry.string_offset);
        LOG_DEBUG(Loader, "{} -> offset: 0x{:08x}, size: 0x{:08x}", entry_name,
                  entry.offset, entry.size);

        // TODO: it doesn't always need to be ExeFS

        if (entry_name == "main.npdm") {
            LOG_NOT_IMPLEMENTED(Loader, "main.npdm loading");
        } else {
            reader.Seek(entries_offset + entry.offset);
            auto nso_reader = reader.CreateSubReader(entry.size);
            NsoLoader loader(nso_reader, entry_name == "rtld");
            auto process_ = loader.LoadProcess(nso_reader, rom_filename);
            if (process_)
                CHECK_AND_SET_PROCESS(process, process_);
        }
    }

    CHECK_AND_RETURN_PROCESS(process);
}

} // namespace hydra::horizon::loader
