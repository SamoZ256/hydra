#include "core/horizon/loader/nca_loader.hpp"

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/loader/nso_loader.hpp"

#define NINTENDO_LOGO_PATH "logo/NintendoLogo.png"
#define STARTUP_MOVIE_PATH "logo/StartupMovie.gif"

namespace hydra::horizon::loader {

namespace {

enum class NpdmFlags : u8 {
    None = 0,
    Is64BitInstruction = BIT(0),
    AddressSpace32Bit = 0x0 << 1,
    AddressSpace64BitOld = 0x1 << 1,
    AddressSpace32BitNoReserved = 0x2 << 1,
    AddressSpace64Bit = 0x3 << 1,
    OptimizeMemoryAllocation = BIT(4),       // 7.0.0+
    DisableDeviceAddressSpaceMerge = BIT(5), // 11.0.0+
    EnableAliasRegionExtraSize = BIT(6),     // 18.0.0+
    PreventCodeReads = BIT(7),               // 19.0.0+
};
ENABLE_ENUM_BITMASK_OPERATORS(NpdmFlags)

struct NpdmMeta {
    u32 magic;
    u32 signature_key_generation; // 9.0.0+
    u32 _reserved_x8;
    NpdmFlags flags;
    u8 _reserved_xd;
    u8 main_thread_priority;
    u8 main_thread_core_number;
    u32 _reserved_x10;
    u32 system_resource_size; // 3.0.0+
    u32 version;
    u32 main_thread_stack_size;
    char name[0x10];
    u8 product_code[0x10];
    u8 _reserved_x40[0x30];
    u32 aci_offset;
    u32 aci_size;
    u32 acid_offset;
    u32 acid_size;
};

} // namespace

NcaLoader::NcaLoader(const filesystem::ContentArchive& content_archive_)
    : content_archive(content_archive_) {
    filesystem::EntryBase* e;
    auto res = content_archive.GetEntry("code/main.npdm", e);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to load main.npdm: {}", res);
        return;
    }

    auto file = dynamic_cast<filesystem::FileBase*>(e);
    if (!file) {
        LOG_ERROR(Loader, "main.npdm is not a file");
        return;
    }

    auto stream = file->Open(filesystem::FileOpenFlags::Read);
    auto reader = stream.CreateReader();

    const auto meta = reader.Read<NpdmMeta>();

    file->Close(stream);

    ASSERT(meta.magic == make_magic4('M', 'E', 'T', 'A'), Loader,
           "Invalid NPDM meta magic 0x{:08x}", meta.magic);

    // TODO: support 32-bit games
    if (!any(meta.flags & NpdmFlags::Is64BitInstruction)) {
        LOG_WARN(Loader, "32-bit games not supported");
        return;
    }

    LOG_DEBUG(Loader, "Name: {}", meta.name);
    LOG_DEBUG(Loader, "Main thread priority: 0x{:02x}",
              meta.main_thread_priority);
    LOG_DEBUG(Loader, "Main thread core number: {}",
              meta.main_thread_core_number);
    LOG_DEBUG(Loader, "Main thread stack size: 0x{:08x}",
              meta.main_thread_stack_size);
    LOG_DEBUG(Loader, "System resource size: 0x{:08x}",
              meta.system_resource_size);

    name = meta.name;
    main_thread_priority = meta.main_thread_priority;
    main_thread_core_number = meta.main_thread_core_number;
    main_thread_stack_size = meta.main_thread_stack_size;
    system_resource_size = meta.system_resource_size;

    // Nintendo logo
    filesystem::EntryBase* nintendo_logo_entry;
    res = content_archive.GetEntry(NINTENDO_LOGO_PATH, nintendo_logo_entry);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " NINTENDO_LOGO_PATH ": {}", res);
        return;
    }

    nintendo_logo_file =
        dynamic_cast<filesystem::FileBase*>(nintendo_logo_entry);
    if (!nintendo_logo_file) {
        LOG_ERROR(Loader, NINTENDO_LOGO_PATH " is not a file");
        return;
    }

    // Startup movie
    filesystem::EntryBase* startup_movie_entry;
    res = content_archive.GetEntry(STARTUP_MOVIE_PATH, startup_movie_entry);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " STARTUP_MOVIE_PATH ": {}", res);
        return;
    }

    startup_movie_file =
        dynamic_cast<filesystem::FileBase*>(startup_movie_entry);
    if (!startup_movie_file) {
        LOG_ERROR(Loader, STARTUP_MOVIE_PATH " is not a file");
        return;
    }
}

void NcaLoader::LoadProcess(kernel::Process* process) {
    // Title ID
    process->SetTitleID(content_archive.GetTitleID());

    for (const auto& [name, entry] : content_archive.GetEntries()) {
        if (name == "code") {
            auto dir = dynamic_cast<filesystem::Directory*>(entry);
            ASSERT(dir, Loader, "Code is not a directory");
            LoadCode(process, dir);
        } else if (name == "data") {
            const auto res = FILESYSTEM_INSTANCE.AddEntry(
                FS_SD_MOUNT "/rom/romFS", entry, true);
            ASSERT(res == filesystem::FsResult::Success, Loader,
                   "Failed to add romFS entry: {}", res);
        } else if (name == "logo") {
            // Do nothing
        } else {
            LOG_NOT_IMPLEMENTED(Loader, "{}", name);
        }
    }
}

void NcaLoader::LoadCode(kernel::Process* process, filesystem::Directory* dir) {
    // HACK: if rtld is not present, use main as the entry point
    std::string entry_point = "rtld";
    filesystem::EntryBase* e;
    if (dir->GetEntry("rtld", e) == filesystem::FsResult::DoesNotExist)
        entry_point = "main";

    for (const auto& [filename, entry] : dir->GetEntries()) {
        auto file = dynamic_cast<filesystem::FileBase*>(entry);
        ASSERT(file, Loader, "Code entry is not a file");
        if (filename == "main.npdm") {
            // Do nothing
        } else {
            NsoLoader loader(file, filename, filename == entry_point);
            loader.SetMainThreadParams(main_thread_priority,
                                       main_thread_core_number,
                                       main_thread_stack_size);
            loader.LoadProcess(process);
        }
    }

    process->SetSystemResourceSize(system_resource_size);

    // TODO: ACI and ACID
}

} // namespace hydra::horizon::loader
