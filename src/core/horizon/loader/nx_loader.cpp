#include "core/horizon/loader/nx_loader.hpp"

#include "core/horizon/filesystem/romfs/romfs.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/loader/npdm.hpp"
#include "core/horizon/loader/nso_loader.hpp"
#include "core/system.hpp"

#define NACP_PATH "meta/control.nacp"
#define ICONS_PATH "meta/icons"
#define NINTENDO_LOGO_PATH "loading_screen/NintendoLogo.png"
#define STARTUP_MOVIE_PATH "loading_screen/StartupMovie.gif"

namespace hydra::horizon::loader {

NxLoader::NxLoader(const filesystem::Directory& dir_) : dir{dir_} {
    parseInfo();
    parseNpdm();

    // NACP
    auto res = dir.getFile(NACP_PATH, nacp_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " NACP_PATH ": {}", res);
        return;
    }

    // Icon
    findIcon();

    // Nintendo logo
    res = dir.getFile(NINTENDO_LOGO_PATH, nintendo_logo_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " NINTENDO_LOGO_PATH ": {}", res);
        return;
    }

    // Startup movie
    res = dir.getFile(STARTUP_MOVIE_PATH, startup_movie_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " STARTUP_MOVIE_PATH ": {}", res);
        return;
    }

    // ExeFS
    res = dir.getDirectory("exefs", exefs_dir);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to get ExeFS directory: {}", res);

    // RomFS
    res = dir.getEntry("romfs", romfs_entry);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to get RomFS entry: {}", res);
}

void NxLoader::loadProcess(System& system, kernel::Process* process) {
    // Title ID
    process->setTitleId(title_id);

    // ExeFS
    loadCode(system, process, exefs_dir);

    // RomFS
    filesystem::IFile* romfs_file;
    if (romfs_entry->isFile()) {
        // Just set the file directly
        romfs_file = static_cast<filesystem::IFile*>(romfs_entry);
    } else {
        // Build romFS
        filesystem::romfs::RomFS romfs(
            *static_cast<filesystem::Directory*>(romfs_entry));
        romfs_file = romfs.build();
        ASSERT(romfs_file, Loader, "Failed to build romFS");
    }

    const auto res = system.getOs().getFilesystem().addEntry(
        FS_SD_MOUNT "/rom/romFS", romfs_file, true);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to add romFS file: {}", res);
}

void NxLoader::parseInfo() {
    filesystem::IFile* file;
    auto res = dir.getFile("info.toml", file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to load info.toml: {}", res);
        return;
    }

    auto stream = file->open(filesystem::FileOpenFlags::Read);

    std::string content;
    content.resize(stream->getSize());
    stream->readToSpan(std::span(content));
    const auto info = toml::parse_str(content);
    title_id = toml::find<u64>(info, "title_id");

    delete stream;
}

void NxLoader::parseNpdm() {
    filesystem::IFile* file;
    auto res = dir.getFile("exefs/main.npdm", file);
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

    main_thread_priority = meta.main_thread_priority;
    main_thread_core_number = meta.main_thread_core_number;
    main_thread_stack_size = meta.main_thread_stack_size;
    system_resource_size = meta.system_resource_size;
}

namespace {

std::string_view getLanguageIconFilename(LanguageCode code) {
    switch (code) {
    case LanguageCode::Japanese:
        return "Japanese";
    case LanguageCode::AmericanEnglish:
        return "AmericanEnglish";
    case LanguageCode::French:
        return "French";
    case LanguageCode::German:
        return "German";
    case LanguageCode::Italian:
        return "Italian";
    case LanguageCode::Spanish:
        return "Spanish";
    case LanguageCode::Chinese:
        return "Chinese";
    case LanguageCode::Korean:
        return "Korean";
    case LanguageCode::Dutch:
        return "Dutch";
    case LanguageCode::Portuguese:
        return "Portuguese";
    case LanguageCode::Russian:
        return "Russian";
    case LanguageCode::Taiwanese:
        return "Taiwanese";
    case LanguageCode::BritishEnglish:
        return "BritishEnglish";
    case LanguageCode::CanadianFrench:
        return "CanadianFrench";
    case LanguageCode::LatinAmericanSpanish:
        return "LatinAmericanSpanish";
    case LanguageCode::TraditionalChinese:
        return "TraditionalChinese";
    case LanguageCode::SimplifiedChinese:
        return "SimplifiedChinese";
    case LanguageCode::BrazilianPortuguese:
        return "BrazilianPortuguese";
    }
}

} // namespace

void NxLoader::findIcon() {
    // Get the icons directory
    filesystem::Directory* icons_dir;
    auto res = dir.getDirectory(ICONS_PATH, icons_dir);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Loader, "Failed to get " ICONS_PATH ": {}", res);
        return;
    }

    // First, try to get the icon for the desired language
    const auto lang_code = toLanguageCode(CONFIG_INSTANCE.getSystemLanguage());
    const auto filename = getLanguageIconFilename(lang_code);
    res = dir.getFile(filename, icon_file);
    if (res != filesystem::FsResult::Success) {
        // Failed, get any icon
        auto it = icons_dir->getEntries().begin();
        if (it == icons_dir->getEntries().end()) {
            LOG_ERROR(Loader, "Failed to get icon");
            return;
        }

        icon_file = static_cast<filesystem::IFile*>(it->second);
    }
}

void NxLoader::loadCode(System& system, kernel::Process* process,
                        filesystem::Directory* exefs_dir) const {
    // HACK: if rtld is not present, use main as the entry point
    std::string entry_point = "rtld";
    filesystem::IEntry* e;
    if (exefs_dir->getEntry("rtld", e) == filesystem::FsResult::DoesNotExist)
        entry_point = "main";

    for (const auto& [filename, entry] : exefs_dir->getEntries()) {
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
