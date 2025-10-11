#pragma once

namespace hydra::horizon {

namespace filesystem {
class FileBase;
class Filesystem;
} // namespace filesystem

class AvatarImageLoader {
  public:
    void LoadSystemAvatars(filesystem::Filesystem& fs);

    void LoadAvatarImage(const std::string& path,
                         std::vector<u8>& out_data) const;

  private:
    std::map<std::string, filesystem::FileBase*> images;
};

} // namespace hydra::horizon
