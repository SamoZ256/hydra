#pragma once

#include "core/horizon/filesystem/const.hpp"
#include "core/horizon/services/const.hpp"
#include "core/horizon/services/fssrv/const.hpp"

namespace hydra::horizon::services::fssrv {

enum class CreateOption : u32 {
    None = 0,
    BigFile = ZTD_BIT(0),
};
ZTD_ENABLE_ENUM_BITWISE_OPERATORS(CreateOption)

struct TimeStampRaw {
    u64 creation_time;
    u64 modification_time;
    u64 access_time;
    bool is_valid;
    u8 _padding_x19[7];
};

class IFileSystem : public IService {
  public:
    explicit IFileSystem(const std::string_view mount_) : mount{mount_} {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    std::string mount;

    // Commands
    result_t createFile(System* system, CreateOption flags, u64 size,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t deleteFile(System* system,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t createDirectory(System* system,
                             InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t deleteDirectory(System* system,
                             InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t deleteDirectoryRecursively(
        System* system, InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t renameFile(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                        InBuffer<BufferAttr::HipcPointer> in_new_path_buffer);
    result_t getEntryType(System* system,
                          InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                          EntryType* out_entry_type);
    result_t openFile(RequestContext* ctx, System* system,
                      filesystem::FileOpenFlags flags,
                      InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    result_t openDirectory(RequestContext* ctx, System* system,
                           DirectoryFilterFlags filter_flags,
                           InBuffer<BufferAttr::HipcPointer> in_path_buffer);
    STUB_REQUEST_COMMAND(commit);
    result_t getFreeSpaceSize(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                              u64* out_size);
    result_t getTotalSpaceSize(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                               u64* out_size);
    result_t
    getFileTimeStampRaw(System* system,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                        TimeStampRaw* out_timestamp); // 3.0.0+
};

} // namespace hydra::horizon::services::fssrv
