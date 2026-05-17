#pragma once

#include "core/horizon/filesystem/file.hpp"

namespace hydra::horizon::filesystem {

class MemoryFile : public IFile {
  public:
    MemoryFile(const std::vector<u8>& data_) : data{std::move(data_)} {}
    MemoryFile(u64 size) : data(size) {}

    void Resize(u64 new_size) override { data.resize(new_size); }

    io::IStream* Open([[maybe_unused]] FileOpenFlags flags) override {
        return new io::MemoryStream(data);
    }

    u64 GetSize() const override { return data.size(); }

  private:
    std::vector<u8> data;

    void DeleteImpl() override {
        // Do nothing
    }
};

} // namespace hydra::horizon::filesystem
