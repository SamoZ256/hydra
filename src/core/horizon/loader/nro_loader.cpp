#include "core/horizon/loader/nro_loader.hpp"

#include "core/horizon/const.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/filesystem/host_file.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::loader {

static const std::string ROM_VIRTUAL_PATH =
    FS_SD_MOUNT "/rom.nro"; // TODO: what should this be?

namespace {

enum class NroSectionType {
    Text,
    Ro,
    Data,
};

struct NroSection {
    u32 offset;
    u32 size;
};

struct NroHeader {
    u8 rocrt[16];
    u32 magic;
    u32 version;
    u32 size;
    u32 flags;
    NroSection sections[3];
    u32 bss_size;
    u8 reserved1[4];
    u8 module_id[32];
    u32 dso_handle_offset;
    u8 reserved2[4];
    u32 embedded_offset;
    u32 embedded_size;
    u32 dyn_str_offset;
    u32 dyn_str_size;
    u32 dyn_sym_offset;
    u32 dyn_sym_size;

    const NroSection& GetSection(NroSectionType type) const {
        return sections[static_cast<u32>(type)];
    }
};

} // namespace

NroLoader::NroLoader(StreamReader reader) {
    // Header
    const auto header = reader.Read<NroHeader>();

    // Validate
    ASSERT(header.magic == make_magic4('N', 'R', 'O', '0'), Loader,
           "Invalid NRO magic \"{}\"", header.magic);

    text_offset = header.GetSection(NroSectionType::Text).offset;
    bss_size = header.bss_size;
}

kernel::Process* NroLoader::LoadProcess(StreamReader reader,
                                        const std::string_view rom_filename) {
    // Create executable memory
    usize executable_size = reader.GetSize() + bss_size;
    uptr base;
    auto ptr = KERNEL_INSTANCE.CreateExecutableMemory(
        executable_size, kernel::MemoryPermission::ReadWriteExecute, true,
        base); // TODO: is the permission correct?
    reader.Seek(0);
    reader.ReadPtr(reinterpret_cast<u8*>(ptr), reader.GetSize());

    // Args
    const u64 argv_offset = executable_size;

    std::string args = fmt::format("\"{}\"", ROM_VIRTUAL_PATH);
    for (const auto& arg : CONFIG_INSTANCE.GetProcessArgs().Get())
        args += fmt::format(" \"{}\"", arg);

    char* argv = reinterpret_cast<char*>(ptr + argv_offset);
    memcpy(argv, args.c_str(), args.size());
    argv[args.size()] = '\0';

    // Config
    const uptr config_offset = argv_offset + args.size() + 1;

#define ADD_ENTRY(t, f, value0, value1)                                        \
    {                                                                          \
        entry->type = ConfigEntryType::t;                                      \
        entry->flags = ConfigEntryFlag::f;                                     \
        entry->values[0] = value0;                                             \
        entry->values[1] = value1;                                             \
        entry++;                                                               \
    }
#define ADD_ENTRY_MANDATORY(t, value0, value1)                                 \
    ADD_ENTRY(t, None, value0, value1)
#define ADD_ENTRY_NON_MANDATORY(t, value0, value1)                             \
    ADD_ENTRY(t, IsMandatory, value0, value1)

    // Entries
    ConfigEntry* entry = reinterpret_cast<ConfigEntry*>(ptr + config_offset);

    ADD_ENTRY_MANDATORY(MainThreadHandle, 0x0000000f,
                        0); // TODO: what thread handle should be used?
    ADD_ENTRY_MANDATORY(Argv, 0, base + argv_offset);
    // TODO: supply the actual availability
    ADD_ENTRY_MANDATORY(SyscallAvailableHint, UINT64_MAX, UINT64_MAX);
    ADD_ENTRY_MANDATORY(SyscallAvailableHint2, UINT64_MAX, 0);
    ADD_ENTRY_MANDATORY(EndOfList, 0, 0);

#undef ADD_ENTRY_NON_MANDATORY
#undef ADD_ENTRY_MANDATORY
#undef ADD_ENTRY

    // filesystem
    const auto res = FILESYSTEM_INSTANCE.AddEntry(
        ROM_VIRTUAL_PATH,
        new filesystem::HostFile(rom_filename, reader.GetSize(),
                                 reader.GetOffset(), false));
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to add romFS entry: {}", res);

    // Process
    kernel::Process* process = new kernel::Process();
    auto& main_thread = process->GetMainThread();
    main_thread.handle->SetEntryPoint(base + sizeof(NroHeader) + text_offset);
    main_thread.handle->SetArg(0, base + config_offset);
    main_thread.handle->SetArg(1, UINT64_MAX);

    return process;
}

} // namespace hydra::horizon::loader
