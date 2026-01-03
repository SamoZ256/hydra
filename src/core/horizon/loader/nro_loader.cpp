#include "core/horizon/loader/nro_loader.hpp"

#include "core/horizon/const.hpp"
#include "core/horizon/filesystem/file_view.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::loader {

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

NroLoader::NroLoader(filesystem::IFile* file_, const bool is_entry_point_)
    : file{file_}, is_entry_point{is_entry_point_} {
    auto stream = file->Open(filesystem::FileOpenFlags::Read);

    // Header
    const auto header = stream->Read<NroHeader>();

    // Validate
    ASSERT_THROWING(header.magic == make_magic4('N', 'R', 'O', '0'), Loader,
                    Error::InvalidMagic, "Invalid NRO magic \"{}\"",
                    header.magic);

    size = header.size;
    text_offset = header.GetSection(NroSectionType::Text).offset;
    bss_size = header.bss_size;

    delete stream;
}

void NroLoader::LoadProcess(kernel::Process* process) {
    auto stream = file->Open(filesystem::FileOpenFlags::Read);

    // Create executable memory
    executable_size = stream->GetSize() + bss_size;
    // TODO: module name
    executable_ptr = process->CreateExecutableMemory(
        "main.nro", executable_size, kernel::MemoryPermission::ReadWriteExecute,
        true,
        executable_base); // TODO: is the permission correct?
    stream->SeekTo(0);
    stream->ReadToSpan(
        std::span(reinterpret_cast<u8*>(executable_ptr), stream->GetSize()));

    // Debug symbols
    // TODO

    delete stream;

    // Main thread
    if (is_entry_point) {
        // TODO: implement?
        LOG_FATAL(Loader, "NRO loading not implemented");
        /*
        // TODO: don' hardcode
        auto [main_thread, main_thread_id] =
            process->CreateMainThread(0x2c, 0, 0x40000);
        main_thread->SetEntryPoint(base + sizeof(NroHeader) + text_offset);
        // TODO: args
        // main_thread->SetArg(0, base + config_offset);
        // main_thread->SetArg(1, UINT64_MAX);
        */
    }
}

vaddr_t NroLoader::GetEntryPoint() const {
    return executable_base + sizeof(NroHeader) + text_offset;
}

} // namespace hydra::horizon::loader
