#pragma once

#include "core/horizon/filesystem/file_base.hpp"

namespace hydra::horizon::filesystem {

class HostFile : public FileBase {
  public:
    HostFile(const std::string_view host_path_, usize size_ = invalid<usize>(),
             u64 offset = 0, bool is_mutable_ = true);
    ~HostFile() override;

    void Resize(usize new_size) override;

    FileStream Open(FileOpenFlags flags) override;
    void Close(FileStream& stream) override;

    // Getters
    usize GetSize() override;

  private:
    std::string host_path;
    usize size;
    bool is_mutable;

    void DeleteImpl() override;
};

} // namespace hydra::horizon::filesystem
