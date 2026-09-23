#include "core/horizon/loader/plugins/manager.hpp"

namespace hydra::horizon::loader::plugins {

void Manager::refresh() {
    plugins.clear();
    plugins.reserve(CONFIG_INSTANCE.getLoaderPlugins().size());
    for (const auto& plugin_config : CONFIG_INSTANCE.getLoaderPlugins()) {
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

        ZTD_ASSIGN_WITH_ERROR_OR(
            auto plugin,
            Plugin::create(plugin_config.path, plugin_config.options), {
                LOG_ERROR(Loader, "Failed to initialize plugin \"{}\": {}",
                          plugin_config.path, error);
                continue;
            });
        plugins.emplace_back(std::move(plugin));
    }
}

Plugin* Manager::findPluginForFormat(std::string_view format) {
    for (auto& plugin : plugins) {
        if (std::ranges::find(plugin.supported_formats, format) !=
            plugin.supported_formats.end())
            return &plugin;
    }

    return nullptr;
}

} // namespace hydra::horizon::loader::plugins
