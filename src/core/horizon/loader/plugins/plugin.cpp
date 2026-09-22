#include "core/horizon/loader/plugins/plugin.hpp"

#include "core/horizon/loader/nx_loader.hpp"

namespace hydra::horizon::loader::plugins {

namespace {

class StreamAdapter : public ztd::io::IStream {
  public:
    StreamAdapter(Plugin& extension_, void* handle_) noexcept
        : plugin{extension_}, handle{handle_} {}
    ~StreamAdapter() noexcept override { plugin.streamDestroy(handle); }

    u64 getSeek() const noexcept override {
        return plugin.streamGetSeek(handle);
    }
    void seekTo(u64 seek) noexcept override {
        plugin.streamSeekTo(handle, seek);
    }
    void seekBy(u64 offset) noexcept override {
        plugin.streamSeekBy(handle, offset);
    }

    u64 getSize() const noexcept override {
        return plugin.streamGetSize(handle);
    }

    void readRaw(std::span<u8> buffer) noexcept override {
        plugin.streamReadRaw(handle, buffer);
    }

  private:
    Plugin& plugin;
    void* handle;
};

class FileAdapter : public filesystem::IFile {
  public:
    FileAdapter(Plugin& extension_, void* handle_)
        : plugin{extension_}, handle{handle_} {}
    ~FileAdapter() override { plugin.fileDestroy(handle); }

    ztd::io::IStream* open(filesystem::FileOpenFlags flags) override {
        (void)flags;
        return new StreamAdapter(plugin, plugin.fileOpen(handle));
    }

    u64 getSize() const override { return plugin.fileGetSize(handle); }

  private:
    Plugin& plugin;
    void* handle;
};

class Loader : public NxLoader {
  public:
    Loader(Plugin& extension_, void* handle_, const filesystem::Directory& dir)
        : NxLoader(dir), plugin{extension_}, handle{handle_} {}
    ~Loader() override {
        // HACK
        (void)plugin;
        (void)handle;
        // plugin.DestroyLoader(handle);
    }

  private:
    Plugin& plugin;
    void* handle;
};

void addFile(void* plugin, filesystem::Directory* dir,
             api::Slice<const char> path, void* handle) {
    const std::string_view path_str(path.data, path.size);
    const auto res = dir->addEntry(
        path_str, new FileAdapter(*reinterpret_cast<Plugin*>(plugin), handle),
        true);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to add file to \"{}\": {}", path_str, res);
}

} // namespace

std::expected<Plugin, Plugin::Error> Plugin::create(const std::string& path) {
    Plugin plugin;

    plugin.library = dlopen(path.data(), RTLD_LAZY);
    if (plugin.library == nullptr)
        return std::unexpected(Error::LoadFailed);

    // Functions
    plugin.get_api_version = plugin.loadFunction<api::Function::GetApiVersion,
                                                 api::GetApiVersionFnT>();
    plugin.query_fn =
        plugin.loadFunction<api::Function::Query, api::QueryFnT>();
    plugin.create_context = plugin.loadFunction<api::Function::CreateContext,
                                                api::CreateContextFnT>();
    plugin.destroy_context = plugin.loadFunction<api::Function::DestroyContext,
                                                 api::DestroyContextFnT>();
    plugin.create_loader_from_file =
        plugin.loadFunction<api::Function::CreateLoaderFromFile,
                            api::CreateLoaderFromFileFnT>();
    plugin.loader_destroy = plugin.loadFunction<api::Function::LoaderDestroy,
                                                api::LoaderDestroyFnT>();
    plugin.file_destroy =
        plugin.loadFunction<api::Function::FileDestroy, api::FileDestroyFnT>();
    plugin.file_open =
        plugin.loadFunction<api::Function::FileOpen, api::FileOpenFnT>();
    plugin.file_get_size =
        plugin.loadFunction<api::Function::FileGetSize, api::FileGetSizeFnT>();
    plugin.stream_destroy = plugin.loadFunction<api::Function::StreamDestroy,
                                                api::StreamDestroyFnT>();
    plugin.stream_get_seek = plugin.loadFunction<api::Function::StreamGetSeek,
                                                 api::StreamGetSeekFnT>();
    plugin.stream_seek_to =
        plugin
            .loadFunction<api::Function::StreamSeekTo, api::StreamSeekToFnT>();
    plugin.stream_seek_by =
        plugin
            .loadFunction<api::Function::StreamSeekBy, api::StreamSeekByFnT>();
    plugin.stream_get_size = plugin.loadFunction<api::Function::StreamGetSize,
                                                 api::StreamGetSizeFnT>();
    plugin.stream_read_raw = plugin.loadFunction<api::Function::StreamReadRaw,
                                                 api::StreamReadRawFnT>();

    // API version
    if (plugin.getApiVersion() != 1)
        return std::unexpected(Error::UnsupportedApiVersion);

    // Info
    plugin.name = plugin.queryString(api::QueryType::Name);
    plugin.display_version = plugin.queryString(api::QueryType::DisplayVersion);
    plugin.supported_formats = split<std::string_view>(
        plugin.queryString(api::QueryType::SupportedFormats), ',');
    const auto api_option_configs_buffer =
        plugin.query(api::QueryType::OptionConfigs);
    const auto api_option_configs =
        std::span(reinterpret_cast<const api::OptionConfig*>(
                      api_option_configs_buffer.data()),
                  api_option_configs_buffer.size() / sizeof(api::OptionConfig));
    plugin.option_configs.reserve(api_option_configs.size());
    for (const auto& api_config : api_option_configs) {
        OptionConfig config{
            .name = std::string_view(api_config.name),
            .description = std::string_view(api_config.description),
            .type = api_config.type,
            .is_required = api_config.is_required,
        };

        switch (api_config.type) {
        case api::OptionType::Enumeration:
            config.enum_value_names = split<std::string_view>(
                std::string_view(api_config.enum_value_names), ',');
            break;
        case api::OptionType::Path:
            config.path_content_types = split<std::string_view>(
                std::string_view(api_config.path_content_types), ',');
            break;
        default:
            break;
        }

        plugin.option_configs.emplace_back(std::move(config));
    }

    LOG_INFO(Loader,
             "Loaded plugin \"{}\" (version: {}, formats: {}) at path \"{}\"",
             plugin.name, plugin.display_version, plugin.supported_formats,
             path);

    return std::move(plugin);
}

std::expected<Plugin, Plugin::Error>
Plugin::create(const std::string& path,
               const std::map<std::string, std::string>& options) {
    return create(path).and_then(
        [=](Plugin plugin) -> std::expected<Plugin, Error> {
            // Verify that all required options are present
            for (const auto& config : plugin.option_configs) {
                if (config.is_required &&
                    !options.contains(std::string(config.name)))
                    return std::unexpected(Error::InvalidOptions);
            }

            // Create context
            ZTD_ASSIGN_OR_RETURN_ERROR(plugin.context,
                                       plugin.createContext(options));

            return plugin;
        });
}

Plugin::~Plugin() {
    if (context != nullptr)
        destroyContext();
    if (library != nullptr)
        dlclose(library);
}

std::optional<NxLoader*> Plugin::load(std::string_view path) {
    const auto root_dir = new filesystem::Directory();
    return createLoaderFromFile(root_dir, path)
        .transform([root_dir, this](void* handle) {
            return new Loader(*this, handle, *root_dir);
        });
}

u64 Plugin::getApiVersion() { return get_api_version(); }

std::span<const u8> Plugin::query(api::QueryType what) {
    return query_fn(what);
}

std::string_view Plugin::queryString(api::QueryType what) {
    const auto buffer = query(what);
    return {reinterpret_cast<const char*>(buffer.data()), buffer.size()};
}

std::expected<void*, Plugin::Error>
Plugin::createContext(const std::map<std::string, std::string>& options) {
    std::vector<api::Option> options_vec;
    options_vec.reserve(options.size());
    for (const auto& [key, value] : options) {
        options_vec.emplace_back(
            api::Slice<const char>(std::string_view(key)),
            api::Slice<const char>(std::string_view(value)));
    }
    const auto ret =
        create_context(api::Slice(std::span<const api::Option>(options_vec)));
    if (ret.res != api::CreateContextResult::Success || ret.value == nullptr)
        return std::unexpected(Error::ContextCreationFailed);

    return ret.value;
}

void Plugin::destroyContext() { destroy_context(context); }

std::optional<void*>
Plugin::createLoaderFromFile(filesystem::Directory* root_dir,
                             std::string_view path) {
    const auto ret = create_loader_from_file(context, this, addFile, root_dir,
                                             api::Slice(std::span(path)));
    if (ret.res != api::CreateLoaderFromFileResult::Success) {
        return std::nullopt;
    }

    return ret.value;
}

void Plugin::loaderDestroy(void* loader) { loader_destroy(loader); }

void Plugin::fileDestroy(void* file) { file_destroy(file); }

void* Plugin::fileOpen(void* file) { return file_open(file); }

u64 Plugin::fileGetSize(void* file) { return file_get_size(file); }

void Plugin::streamDestroy(void* stream) { stream_destroy(stream); }

u64 Plugin::streamGetSeek(void* stream) { return stream_get_seek(stream); }

void Plugin::streamSeekTo(void* stream, u64 offset) {
    stream_seek_to(stream, offset);
}

void Plugin::streamSeekBy(void* stream, u64 offset) {
    stream_seek_by(stream, offset);
}

u64 Plugin::streamGetSize(void* stream) { return stream_get_size(stream); }

void Plugin::streamReadRaw(void* stream, std::span<u8> buffer) {
    stream_read_raw(stream, api::Slice(buffer));
}

} // namespace hydra::horizon::loader::plugins
