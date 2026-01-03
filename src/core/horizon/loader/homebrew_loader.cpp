#include "core/horizon/loader/homebrew_loader.hpp"

#include <random>

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/const.hpp"
#include "core/horizon/filesystem/disk_file.hpp"
#include "core/horizon/filesystem/file_view.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/kernel/kernel.hpp"

namespace hydra::horizon::loader {

namespace {

constexpr usize STACK_MEMORY_SIZE = 0x40000;

enum class ConfigEntryType : u32 {
    EndOfList = 0,
    MainThreadHandle = 1,
    NextLoadPath = 2,
    OverrideHeap = 3,
    OverrideService = 4,
    Argv = 5,
    SyscallAvailableHint = 6,
    AppletType = 7,
    AppletWorkaround = 8,
    Reserved9 = 9,
    ProcessHandle = 10,
    LastLoadResult = 11,
    RandomSeed = 14,
    UserIdStorage = 15,
    HosVersion = 16,
    SyscallAvailableHint2 = 17,
};

enum class ConfigEntryFlag : u32 {
    None = 0,
    IsMandatory = BIT(0),
};

struct ConfigEntry {
    ConfigEntryType type;
    ConfigEntryFlag flags;
    u64 values[2];
};

class HomebrewThread : public kernel::GuestThread {
  public:
    // TODO: don't hardcode priority
    HomebrewThread(kernel::Process* process, std::string_view path_)
        : kernel::GuestThread(
              process, kernel::STACK_REGION.begin + STACK_MEMORY_SIZE - 0x10,
              0x2c, "Homebrew thread"),
          path{path_} {}

  protected:
    void Run() override {
        // Process handle
        const auto self_process_handle = process->AddHandleNoRetain(process);

        // State
        static constexpr char NOTICE_TEXT[] =
            "Hydra Nintendo Switch emulator - Homebrew loader";

        static constexpr usize NOTICE_TEXT_SIZE = 0x100;
        static constexpr usize RETURN_ADDRESS_SIZE = sizeof(u32);
        static constexpr usize USER_ID_STORAGE_SIZE = sizeof(u128);
        static constexpr usize ARGV_SIZE = 0x800;
        static constexpr usize NEXT_LOAD_PATH_SIZE = 0x200;

        static constexpr u32 NOTICE_TEXT_OFFSET = 0x0;
        static constexpr u32 RETURN_ADDRESS_OFFSET =
            NOTICE_TEXT_OFFSET + NOTICE_TEXT_SIZE;
        static constexpr u32 USER_ID_STORAGE_OFFSET =
            RETURN_ADDRESS_OFFSET + RETURN_ADDRESS_SIZE;
        static constexpr u32 ARGV_OFFSET =
            USER_ID_STORAGE_OFFSET + USER_ID_STORAGE_SIZE;
        static constexpr u32 NEXT_LOAD_PATH_OFFSET = ARGV_OFFSET + ARGV_SIZE;
        static constexpr u32 NEXT_ARGV_OFFSET =
            NEXT_LOAD_PATH_OFFSET + ARGV_SIZE;

        // TODO: memory type
        // TODO: region
        vaddr_t state_base;
        auto state_ptr = process->CreateMemory(
            kernel::EXECUTABLE_REGION, ARGV_SIZE * 2 + NEXT_LOAD_PATH_SIZE,
            static_cast<kernel::MemoryType>(4),
            kernel::MemoryPermission::ReadWrite, true, state_base);

        // Notice text
        {
            auto notice_ptr =
                reinterpret_cast<char*>(state_ptr + NOTICE_TEXT_OFFSET);
            memcpy(notice_ptr, NOTICE_TEXT, sizeof(NOTICE_TEXT));
        }

        // Return address
        *reinterpret_cast<u32*>(state_ptr + RETURN_ADDRESS_OFFSET) =
            0xd4000141; // svc 0x0a (svcExitThread)

        // User ID storage
        {
            auto user_id_ptr =
                reinterpret_cast<u128*>(state_ptr + USER_ID_STORAGE_OFFSET);
            *user_id_ptr = CONFIG_INSTANCE.GetUserId();
        }

        // Argv
        {
            std::string argv = fmt::format("\"{}\"", path);
            for (const auto& arg : CONFIG_INSTANCE.GetProcessArgs())
                argv += fmt::format(" \"{}\"", arg);

            auto argv_ptr = reinterpret_cast<char*>(state_ptr + ARGV_OFFSET);
            memcpy(argv_ptr, argv.data(), argv.size());
            argv_ptr[argv.size()] = '\0';
        }

        while (true) {
            LOG_INFO(Loader, "Running Homebrew: {} (argv: {})", path,
                     reinterpret_cast<const char*>(state_ptr + ARGV_OFFSET));

            // File
            filesystem::IFile* file;
            const auto res =
                KERNEL_INSTANCE.GetFilesystem().GetFile(path, file);
            ASSERT(res == filesystem::FsResult::Success, Loader,
                   "Failed to get Homebrew file: {}", res);

            // NRO loader
            {
                NroLoader nro_loader(file, false);
                nro_loader.LoadProcess(process);
                const auto executable_ptr = nro_loader.GetExecutablePtr();

                // Random
                std::random_device rd;
                std::mt19937_64 gen(rd());

                // Config
                const uptr config_offset = nro_loader.GetExecutableSize();

#define ADD_ENTRY(t, f, value0, value1)                                        \
    {                                                                          \
        entry->type = ConfigEntryType::t;                                      \
        entry->flags = ConfigEntryFlag::f;                                     \
        entry->values[0] = value0;                                             \
        entry->values[1] = value1;                                             \
        entry++;                                                               \
    }
#define ADD_ENTRY_OPTIONAL(t, value0, value1) ADD_ENTRY(t, None, value0, value1)
#define ADD_ENTRY_MANDATORY(t, value0, value1)                                 \
    ADD_ENTRY(t, IsMandatory, value0, value1)

                // Entries
                ConfigEntry* entry = reinterpret_cast<ConfigEntry*>(
                    executable_ptr + config_offset);

                ADD_ENTRY_OPTIONAL(MainThreadHandle, self_handle, 0);
                ADD_ENTRY_OPTIONAL(ProcessHandle, self_process_handle, 0);
                ADD_ENTRY_OPTIONAL(
                    AppletType,
                    static_cast<u64>(kernel::AppletType::Application), 0);
                // TODO: override heap?
                ADD_ENTRY_OPTIONAL(Argv, 0, state_base + ARGV_OFFSET);
                ADD_ENTRY_OPTIONAL(NextLoadPath,
                                   state_base + NEXT_LOAD_PATH_OFFSET,
                                   state_base + NEXT_ARGV_OFFSET);
                // TODO: load last result
                ADD_ENTRY_OPTIONAL(SyscallAvailableHint,
                                   std::numeric_limits<u64>::max(),
                                   std::numeric_limits<u64>::max());
                ADD_ENTRY_OPTIONAL(SyscallAvailableHint2,
                                   std::numeric_limits<u64>::max(), 0);
                ADD_ENTRY_OPTIONAL(RandomSeed, gen(), gen());
                ADD_ENTRY_OPTIONAL(UserIdStorage,
                                   state_base + USER_ID_STORAGE_OFFSET, 0);
                ADD_ENTRY_OPTIONAL(HosVersion,
                                   BIT(31) | (FIRMWARE_VERSION.major << 16) |
                                       (FIRMWARE_VERSION.minor << 8) |
                                       FIRMWARE_VERSION.micro,
                                   0x41544d4f53504852ul); // "ATMOSPHR"
                ADD_ENTRY_OPTIONAL(EndOfList, state_base + NOTICE_TEXT_OFFSET,
                                   sizeof(NOTICE_TEXT));

#undef ADD_ENTRY_NON_MANDATORY
#undef ADD_ENTRY_MANDATORY
#undef ADD_ENTRY

                // Params
                entry_point = nro_loader.GetEntryPoint();
                return_address = state_base + RETURN_ADDRESS_OFFSET;
                args[0] = nro_loader.GetExecutableBase() + config_offset;
                args[1] = std::numeric_limits<u64>::max();
            }

            // Run
            kernel::GuestThread::Run();

            // Next load
            path = std::string(reinterpret_cast<const char*>(
                state_ptr + NEXT_LOAD_PATH_OFFSET));
            if (path.empty())
                break;

            memset(reinterpret_cast<void*>(state_ptr + NEXT_LOAD_PATH_OFFSET),
                   0, NEXT_LOAD_PATH_SIZE);

            // Copy argv
            memcpy(reinterpret_cast<void*>(state_ptr + ARGV_OFFSET),
                   reinterpret_cast<void*>(state_ptr + NEXT_ARGV_OFFSET),
                   ARGV_SIZE);

            // HACK: since the program exited by calling svcExitThread, we need
            // to reset the thread
            Reset();
        }
    }

  private:
    std::string path;
    handle_id_t self_handle{INVALID_HANDLE_ID};

  public:
    SETTER(self_handle, SetSelfHandle);
};

} // namespace

HomebrewLoader::HomebrewLoader(filesystem::IFile* file_)
    : file{file_}, nro_loader(file, false) {
    // Asset section
    const auto asset_begin = nro_loader.GetSize();
    TryLoadAssetSection(new filesystem::FileView(file, asset_begin));
}

void HomebrewLoader::LoadProcess(kernel::Process* process) {
    // Get name
    auto stream = nacp_file->Open(filesystem::FileOpenFlags::Read);

    const auto nacp = stream->Read<services::ns::ApplicationControlProperty>();
    std::string title_name = nacp.GetApplicationTitle().name;
    std::replace(title_name.begin(), title_name.end(), ' ', '_');

    delete stream;

    // Map file
    std::string mapped_path =
        fmt::format(FS_SD_MOUNT "/switch/{}/{}.nro", title_name, title_name);
    const auto res =
        KERNEL_INSTANCE.GetFilesystem().AddEntry(mapped_path, file, true);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to map Homebrew file: {}", res);

    // Stack memory
    process->CreateStackMemory(STACK_MEMORY_SIZE);

    // Main thread
    auto main_thread = new HomebrewThread(process, mapped_path);
    const auto main_thread_handle_id = process->SetMainThread(main_thread);
    main_thread->SetSelfHandle(main_thread_handle_id);
}

namespace {

struct AssetSection {
    u64 offset;
    u64 size;
};

struct AssetHeader {
    u32 magic;
    u32 format_version;
    AssetSection icon_section;
    AssetSection nacp_section;
    AssetSection romfs_section;
};

} // namespace

void HomebrewLoader::TryLoadAssetSection(filesystem::IFile* file) {
    auto stream = file->Open(filesystem::FileOpenFlags::Read);

    // Header
    const auto header = stream->Read<AssetHeader>();
    // TODO: is this the correct way to check if the asset section is present?
    if (header.magic != make_magic4('A', 'S', 'E', 'T')) {
        LOG_WARN(Loader, "Asset section not found");
        return;
    }

    // Icon
    if (header.icon_section.size > 0)
        icon_file = new filesystem::FileView(file, header.icon_section.offset,
                                             header.icon_section.size);

    // NACP
    if (header.nacp_section.size > 0)
        nacp_file = new filesystem::FileView(file, header.nacp_section.offset,
                                             header.nacp_section.size);

    delete stream;
}

} // namespace hydra::horizon::loader
