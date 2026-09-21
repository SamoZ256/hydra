#pragma once

#include "core/horizon/filesystem/file.hpp"

namespace hydra::horizon::filesystem {

class MemoryFile : public IFile {
  public:
    explicit MemoryFile(std::vector<u8> data_) : data{std::move(data_)} {}
    explicit MemoryFile(u64 size) : data(size) {}

    void resize(u64 new_size) override { data.resize(new_size); }

    ztd::io::IStream* open([[maybe_unused]] FileOpenFlags flags) override {
        return new ztd::io::MemoryStream(data);
    }

    u64 getSize() const override { return data.size(); }

  private:
    std::vector<u8> data;

    void deleteImpl() override {
        // Do nothing
    }
};

} // namespace hydra::horizon::filesystem
