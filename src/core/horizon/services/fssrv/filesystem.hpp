#pragma once

#include "core/horizon/filesystem/const.hpp"
#include "core/horizon/services/const.hpp"
#include "core/horizon/services/fssrv/const.hpp"

namespace hydra::horizon::services::fssrv {

enum class CreateOption : u32 {
    None = 0,
    BigFile = BIT(0),
};
ENABLE_ENUM_BITWISE_OPERATORS(CreateOption)

struct TimeStampRaw {
    u64 creation_time;
    u64 modification_time;
    u64 access_time;
    bool is_valid;
    u8 _padding_x19[7];
};

class IFileSystem : public IService {
  public:
    IFileSystem(const std::string_view mount_) : mount{mount_} {}

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    std::string mount;

    // Commands
    result_t CreateFile(System* system, CreateOption flags, u64 size,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t DeleteFile(System* system,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t CreateDirectory(System* system,
                             InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t DeleteDirectory(System* system,
                             InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t DeleteDirectoryRecursively(
        System* system, InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t RenameFile(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                        InBuffer<BufferAttr::HipcPointer> in_new_path_buffer);
    result_t GetEntryType(System* system,
                          InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                          EntryType* out_entry_type);
    result_t OpenFile(RequestContext* ctx, System* system,
                      filesystem::FileOpenFlags flags,
                      InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t OpenDirectory(RequestContext* ctx, System* system,
                           DirectoryFilterFlags filter_flags,
                           InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    STUB_REQUEST_COMMAND(Commit);
    result_t GetFreeSpaceSize(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                              u64* out_size);
    result_t GetTotalSpaceSize(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                               u64* out_size);
    result_t
    GetFileTimeStampRaw(System* system,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                        TimeStampRaw* out_timestamp); // 3.0.0+
};

} // namespace hydra::horizon::services::fssrv
