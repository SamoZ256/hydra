#pragma once

#include "core/horizon/filesystem/const.hpp"
#include "core/horizon/services/const.hpp"
#include "core/horizon/services/fssrv/const.hpp"

namespace hydra::horizon::services::fssrv {

enum class CreateOption : u32 {
    None = 0,
    BigFile = BIT(0),
};
ENABLE_ENUM_BITMASK_OPERATORS(CreateOption)

class IFileSystem : public ServiceBase {
  public:
    IFileSystem(const std::string_view mount_) : mount{mount_} {}

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

    usize GetPointerBufferSize() override {
        // TODO: what should be returned
        return 0x400;
    }

  private:
    std::string mount;

    // Commands
    result_t CreateFile(CreateOption flags, u64 size,
                        InBuffer<BufferAttr::HipcPointer> path_buffer);
    result_t DeleteFile(InBuffer<BufferAttr::HipcPointer> path_buffer);
    result_t CreateDirectory(InBuffer<BufferAttr::HipcPointer> path_buffer);
    result_t DeleteDirectory(InBuffer<BufferAttr::HipcPointer> path_buffer);
    result_t
    DeleteDirectoryRecursively(InBuffer<BufferAttr::HipcPointer> path_buffer);
    result_t GetEntryType(InBuffer<BufferAttr::HipcPointer> path_buffer,
                          EntryType* out_entry_type);
    result_t OpenFile(add_service_fn_t add_service,
                      filesystem::FileOpenFlags flags,
                      InBuffer<BufferAttr::HipcPointer> path_buffer);
    result_t OpenDirectory(add_service_fn_t add_service,
                           DirectoryFilterFlags filter_flags,
                           InBuffer<BufferAttr::HipcPointer> path_buffer);
    STUB_REQUEST_COMMAND(Commit);
};

} // namespace hydra::horizon::services::fssrv
