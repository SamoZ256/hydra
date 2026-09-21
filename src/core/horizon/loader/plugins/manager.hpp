#pragma once

#include "core/horizon/loader/plugins/plugin.hpp"

namespace hydra::horizon::loader::plugins {

class Manager {
  public:
    Manager() { refresh(); }

    void refresh();

    Plugin* findPluginForFormat(std::string_view format);

  private:
    std::vector<Plugin> plugins;
};

} // namespace hydra::horizon::loader::plugins
