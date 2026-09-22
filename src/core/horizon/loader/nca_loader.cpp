#include "core/horizon/loader/nca_loader.hpp"

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/loader/npdm.hpp"
#include "core/horizon/loader/nso_loader.hpp"
#include "core/system.hpp"

#define NINTENDO_LOGO_PATH "logo/NintendoLogo.png"
#define STARTUP_MOVIE_PATH "logo/StartupMovie.gif"

namespace hydra::horizon::loader {

NcaLoader::NcaLoader(filesystem::ContentArchive content_archive_)
    : content_archive{std::move(content_archive_)} {
    // Nintendo logo
    auto res = content_archive.getFile(NINTENDO_LOGO_PATH, nintendo_logo_file);
    if (res != filesystem::FsResult::Success)
        LOG_WARN(Loader, "Failed to get " NINTENDO_LOGO_PATH ": {}", res);

    // Startup movie
    res = content_archive.getFile(STARTUP_MOVIE_PATH, startup_movie_file);
    if (res != filesystem::FsResult::Success)
        LOG_WARN(Loader, "Failed to get " STARTUP_MOVIE_PATH ": {}", res);

    // ExeFS
    res = content_archive.getDirectory("code", exefs_dir);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to get ExeFS directory: {}", res);

    // NPDM
    filesystem::IFile* file;
    res = content_archive.getFile("code/main.npdm", file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to load main.npdm: {}", res);
        return;
    }

    auto stream = file->open(filesystem::FileOpenFlags::Read);

    const auto meta = stream->read<NpdmMeta>();

    delete stream;

    ASSERT(meta.magic == makeMagic4('M', 'E', 'T', 'A'), Loader,
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

    // RomFS
    filesystem::IFile* romfs_file = nullptr;
    res = content_archive.getFile("data", romfs_file);
    // TODO: why doesn't Animal Well have romFS?
    if (res != filesystem::FsResult::Success)
        LOG_WARN(Loader, "Failed to get romFS file: {}", res);
    romfs_entry = romfs_file;
}

void NcaLoader::loadProcess(System& system, kernel::Process* process) {
    // Title ID
    process->setTitleId(content_archive.getTitleId());

    // ExeFS
    loadCode(system, process, exefs_dir);

    // RomFS
    const auto res = system.getOs().getFilesystem().addEntry(
        FS_SD_MOUNT "/rom/romFS", romfs_entry, true);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to add romFS file: {}", res);
}

void NcaLoader::loadCode(System& system, kernel::Process* process,
                         filesystem::Directory* dir) const {
    // HACK: if rtld is not present, use main as the entry point
    std::string entry_point = "rtld";
    filesystem::IEntry* e;
    if (dir->getEntry("rtld", e) == filesystem::FsResult::DoesNotExist)
        entry_point = "main";

    for (const auto& [filename, entry] : dir->getEntries()) {
        ASSERT(entry->isFile(), Loader, "Code entry is not a file");
        auto file = static_cast<filesystem::IFile*>(entry);
        if (filename == "main.npdm") {
            // Do nothing
        } else {
            LOG_DEBUG(Loader, "Loading {}", filename);
            NsoLoader loader(file, filename, filename == entry_point);
            loader.setMainThreadParams(main_thread_priority,
                                       main_thread_core_number,
                                       main_thread_stack_size);
            loader.loadProcess(system, process);
        }
    }

    process->setSystemResourceSize(system_resource_size);

    // TODO: ACI and ACID
}

} // namespace hydra::horizon::loader
