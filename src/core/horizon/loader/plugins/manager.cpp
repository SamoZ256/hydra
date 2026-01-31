#include "core/horizon/loader/plugins/manager.hpp"

namespace hydra::horizon::loader::plugins {

void Manager::Refresh() {
    plugins.clear();
    plugins.reserve(CONFIG_INSTANCE.GetLoaderPlugins().size());
    for (const auto& plugin_config : CONFIG_INSTANCE.GetLoaderPlugins()) {
        if (!std::filesystem::exists(plugin_config.path)) {
            LOG_ERROR(Other, "Plugin path \"{}\" does not exist",
                      plugin_config.path);
            continue;
        }
        if (!std::filesystem::is_regular_file(plugin_config.path)) {
            LOG_ERROR(Other, "Plugin path \"{}\" is not a file",
                      plugin_config.path);
            continue;
        }

        try {
            plugins.emplace_back(plugin_config.path, plugin_config.options);
        } catch (Plugin::Error err) {
            // TODO: error popup?
        } catch (Plugin::ContextError err) {
            // TODO: error popup?
        }
    }
}

Plugin* Manager::FindPluginForFormat(std::string_view format) {
    for (auto& plugin : plugins) {
        if (std::find(plugin.supported_formats.begin(),
                      plugin.supported_formats.end(),
                      format) != plugin.supported_formats.end())
            return &plugin;
    }

    return nullptr;
}

} // namespace hydra::horizon::loader::plugins
