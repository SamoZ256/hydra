#include "core/horizon/loader/nx_loader.hpp"

#include "core/horizon/filesystem/romfs/romfs.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/loader/npdm.hpp"
#include "core/horizon/loader/nso_loader.hpp"

#define NACP_PATH "meta/control.nacp"
#define ICON_PATH "meta/icons/AmericanEnglish.jpg"
#define NINTENDO_LOGO_PATH "loading_screen/NintendoLogo.png"
#define STARTUP_MOVIE_PATH "loading_screen/StartupMovie.gif"

namespace hydra::horizon::loader {

NxLoader::NxLoader(const filesystem::Directory& dir_) : dir{dir_} {
    ParseInfo();
    ParseNpdm();

    // NACP
    auto res = dir.GetFile(NACP_PATH, nacp_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " NACP_PATH ": {}", res);
        return;
    }

    // Icon
    res = dir.GetFile(ICON_PATH, icon_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " ICON_PATH ": {}", res);
        return;
    }

    // Nintendo logo
    res = dir.GetFile(NINTENDO_LOGO_PATH, nintendo_logo_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " NINTENDO_LOGO_PATH ": {}", res);
        return;
    }

    // Startup movie
    res = dir.GetFile(STARTUP_MOVIE_PATH, startup_movie_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " STARTUP_MOVIE_PATH ": {}", res);
        return;
    }
}

void NxLoader::LoadProcess(kernel::Process* process) {
    // Title ID
    process->SetTitleID(title_id);

    // ExeFS
    filesystem::Directory* exefs_dir;
    auto res = dir.GetDirectory("exefs", exefs_dir);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to get ExeFS directory: {}", res);
    LoadCode(process, exefs_dir);

    // RomFS

    // Get entry
    filesystem::IEntry* romfs_entry;
    res = dir.GetEntry("romfs", romfs_entry);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to get RomFS entry: {}", res);

    filesystem::IFile* romfs_file;
    if (romfs_entry->IsFile()) {
        // Just set the file directly
        romfs_file = static_cast<filesystem::IFile*>(romfs_entry);
    } else {
        // Build romFS
        filesystem::romfs::RomFS romfs(
            *static_cast<filesystem::Directory*>(romfs_entry));
        romfs_file = romfs.Build();
        ASSERT(romfs_file, Loader, "Failed to build romFS");
    }

    // Add to filesystem
    res = KERNEL_INSTANCE.GetFilesystem().AddEntry(FS_SD_MOUNT "/rom/romFS",
                                                   romfs_file, true);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to add romFS file: {}", res);
}

void NxLoader::ParseInfo() {
    filesystem::IFile* file;
    auto res = dir.GetFile("info.toml", file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to load info.toml: {}", res);
        return;
    }

    auto stream = file->Open(filesystem::FileOpenFlags::Read);

    std::string content;
    content.resize(stream->GetSize());
    stream->ReadToSpan(std::span(content));
    const auto info = toml::parse_str(content);
    title_id = toml::find<u64>(info, "title_id");

    delete stream;
}

void NxLoader::ParseNpdm() {
    filesystem::IFile* file;
    auto res = dir.GetFile("exefs/main.npdm", file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to load main.npdm: {}", res);
        return;
    }

    auto stream = file->Open(filesystem::FileOpenFlags::Read);

    const auto meta = stream->Read<NpdmMeta>();

    delete stream;

    ASSERT_THROWING(meta.magic == make_magic4('M', 'E', 'T', 'A'), Loader,
                    Error::InvalidNpdmMagic, "Invalid NPDM meta magic 0x{:08x}",
                    meta.magic);

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

    main_thread_priority = meta.main_thread_priority;
    main_thread_core_number = meta.main_thread_core_number;
    main_thread_stack_size = meta.main_thread_stack_size;
    system_resource_size = meta.system_resource_size;
}

void NxLoader::LoadCode(kernel::Process* process, filesystem::Directory* dir) {
    // HACK: if rtld is not present, use main as the entry point
    std::string entry_point = "rtld";
    filesystem::IEntry* e;
    if (dir->GetEntry("rtld", e) == filesystem::FsResult::DoesNotExist)
        entry_point = "main";

    for (const auto& [filename, entry] : dir->GetEntries()) {
        auto file = dynamic_cast<filesystem::IFile*>(entry);
        ASSERT(file, Loader, "Code entry is not a file");
        if (filename == "main.npdm") {
            // Do nothing
        } else {
            LOG_DEBUG(Loader, "Loading {}", filename);
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
