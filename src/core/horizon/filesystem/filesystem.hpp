#pragma once

#include "core/horizon/filesystem/device.hpp"

namespace Hydra::Horizon::Filesystem {

class Filesystem {
  public:
    static Filesystem& GetInstance();

    Filesystem();
    ~Filesystem();

    void Mount(const std::string& mount);

    [[nodiscard]] FsResult AddEntry(EntryBase* entry, const std::string& path,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult AddEntry(const std::string& host_path,
                                    const std::string& path,
                                    bool add_intermediate = false);
    [[nodiscard]] FsResult GetEntry(const std::string& path,
                                    EntryBase*& out_entry);

    [[nodiscard]] FsResult GetFile(const std::string& path, File*& out_file);
    [[nodiscard]] FsResult GetDirectory(const std::string& path,
                                        Directory*& out_directory);

  private:
    std::map<std::string, Device> devices;
};

} // namespace Hydra::Horizon::Filesystem
