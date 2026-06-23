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

        (void)Plugin::Create(plugin_config.path, plugin_config.options)
            .transform([this](Plugin plugin) {
                plugins.emplace_back(std::move(plugin));
            });
    }
}

Plugin* Manager::FindPluginForFormat(std::string_view format) {
    for (auto& plugin : plugins) {
        if (std::ranges::find(plugin.supported_formats, format) !=
            plugin.supported_formats.end())
            return &plugin;
    }

    return nullptr;
}

} // namespace hydra::horizon::loader::plugins
