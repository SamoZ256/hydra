#pragma once

#include "core/horizon/loader/plugins/plugin.hpp"

namespace hydra::horizon::loader::plugins {

class Manager {
  public:
    void Refresh();

    Plugin* FindPluginForFormat(std::string_view format);

  private:
    std::vector<Plugin> plugins;
};

} // namespace hydra::horizon::loader::plugins
