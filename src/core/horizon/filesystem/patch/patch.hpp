#pragma once

#include "core/horizon/filesystem/patch/const.hpp"

namespace hydra::horizon::filesystem::patch {

class Patch {
  public:
    Patch(const BuildId& build_id_, const std::string_view name_,
          std::vector<Block>&& blocks_)
        : build_id{build_id_}, name{name_}, blocks{std::move(blocks_)} {}

    void apply(std::span<u8> image) const;

  private:
    BuildId build_id;
    std::string name;
    std::vector<Block> blocks;

  public:
    CONST_REF_GETTER(build_id, getBuildId);
    CONST_REF_GETTER(name, getName);
    CONST_REF_GETTER(blocks, getBlocks);
};

class Collection {
  public:
    void scan(const std::vector<std::string>& paths);
    void apply(const BuildId& build_id, std::span<u8> image) const;

  private:
    std::vector<Patch> patches;

    // Helpers
    void scanFile(const std::filesystem::path& path);

  public:
    CONST_REF_GETTER(patches, getPatches);
};

std::string buildIdToString(const BuildId& build_id);
std::optional<BuildId> buildIdFromString(std::string_view str);

} // namespace hydra::horizon::filesystem::patch
