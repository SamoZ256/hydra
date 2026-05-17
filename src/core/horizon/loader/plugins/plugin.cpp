#include "core/horizon/loader/plugins/plugin.hpp"

#include "core/horizon/loader/nx_loader.hpp"

namespace hydra::horizon::loader::plugins {

namespace {

class StreamAdapter : public io::IStream {
  public:
    StreamAdapter(Plugin& extension_, void* handle_)
        : plugin{extension_}, handle{handle_} {}
    ~StreamAdapter() override { plugin.StreamDestroy(handle); }

    u64 GetSeek() const override { return plugin.StreamGetSeek(handle); }
    void SeekTo(u64 seek) override { plugin.StreamSeekTo(handle, seek); }
    void SeekBy(u64 offset) override { plugin.StreamSeekBy(handle, offset); }

    u64 GetSize() const override { return plugin.StreamGetSize(handle); }

    void ReadRaw(std::span<u8> buffer) override {
        plugin.StreamReadRaw(handle, buffer);
    }

  private:
    Plugin& plugin;
    void* handle;
};

class FileAdapter : public filesystem::IFile {
  public:
    FileAdapter(Plugin& extension_, void* handle_)
        : plugin{extension_}, handle{handle_} {}
    ~FileAdapter() override { plugin.FileDestroy(handle); }

    io::IStream* Open(filesystem::FileOpenFlags flags) override {
        (void)flags;
        return new StreamAdapter(plugin, plugin.FileOpen(handle));
    }

    u64 GetSize() const override { return plugin.FileGetSize(handle); }

  private:
    Plugin& plugin;
    void* handle;
};

class Loader : public NxLoader {
  public:
    Loader(Plugin& extension_, void* handle_, const filesystem::Directory& dir)
        : NxLoader(dir), plugin{extension_}, handle{handle_} {}
    ~Loader() {
        // HACK
        (void)plugin;
        (void)handle;
        // plugin.DestroyLoader(handle);
    }

  private:
    Plugin& plugin;
    void* handle;
};

void AddFile(void* plugin, filesystem::Directory* dir,
             api::Slice<const char> path, void* handle) {
    const std::string_view path_str(path.data, path.size);
    const auto res = dir->AddEntry(
        path_str, new FileAdapter(*reinterpret_cast<Plugin*>(plugin), handle),
        true);
    ASSERT(res == filesystem::FsResult::Success, Loader,
           "Failed to add file to \"{}\": {}", path_str, res);
}

} // namespace

Plugin::Plugin(const std::string& path) {
    library = dlopen(path.data(), RTLD_LAZY);
    ASSERT_THROWING(library, Loader, Error::LoadFailed,
                    "Failed to load plugin at path {}: {}", path, dlerror());

    // Functions
    get_api_version =
        LoadFunction<api::Function::GetApiVersion, api::GetApiVersionFnT>();
    query = LoadFunction<api::Function::Query, api::QueryFnT>();
    create_context =
        LoadFunction<api::Function::CreateContext, api::CreateContextFnT>();
    destroy_context =
        LoadFunction<api::Function::DestroyContext, api::DestroyContextFnT>();
    create_loader_from_file = LoadFunction<api::Function::CreateLoaderFromFile,
                                           api::CreateLoaderFromFileFnT>();
    loader_destroy =
        LoadFunction<api::Function::LoaderDestroy, api::LoaderDestroyFnT>();
    file_destroy =
        LoadFunction<api::Function::FileDestroy, api::FileDestroyFnT>();
    file_open = LoadFunction<api::Function::FileOpen, api::FileOpenFnT>();
    file_get_size =
        LoadFunction<api::Function::FileGetSize, api::FileGetSizeFnT>();
    stream_destroy =
        LoadFunction<api::Function::StreamDestroy, api::StreamDestroyFnT>();
    stream_get_seek =
        LoadFunction<api::Function::StreamGetSeek, api::StreamGetSeekFnT>();
    stream_seek_to =
        LoadFunction<api::Function::StreamSeekTo, api::StreamSeekToFnT>();
    stream_seek_by =
        LoadFunction<api::Function::StreamSeekBy, api::StreamSeekByFnT>();
    stream_get_size =
        LoadFunction<api::Function::StreamGetSize, api::StreamGetSizeFnT>();
    stream_read_raw =
        LoadFunction<api::Function::StreamReadRaw, api::StreamReadRawFnT>();

    // API version
    ASSERT_THROWING(GetApiVersion() == 1, Loader, Error::InvalidApiVersion,
                    "Invalid API version");

    // Info
    name = QueryString(api::QueryType::Name);
    display_version = QueryString(api::QueryType::DisplayVersion);
    supported_formats = split<std::string_view>(
        QueryString(api::QueryType::SupportedFormats), ',');
    const auto api_option_configs_buffer = Query(api::QueryType::OptionConfigs);
    const auto api_option_configs =
        std::span(reinterpret_cast<const api::OptionConfig*>(
                      api_option_configs_buffer.data()),
                  api_option_configs_buffer.size() / sizeof(api::OptionConfig));
    option_configs.reserve(api_option_configs.size());
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

        option_configs.emplace_back(std::move(config));
    }

    LOG_INFO(Loader,
             "Loaded plugin \"{}\" (version: {}, formats: {}) at path \"{}\"",
             name, display_version, supported_formats, path);
}

Plugin::Plugin(const std::string& path,
               const std::map<std::string, std::string>& options)
    : Plugin(path) {
    // Verify that all required options are present
    for (const auto& config : option_configs) {
        if (config.is_required) {
            ASSERT_THROWING(options.contains(std::string(config.name)), Loader,
                            ContextError::InvalidOptions,
                            "Missing option \"{}\"", config.name);
        }
    }

    // Create context
    CreateContext(options);
}

Plugin::~Plugin() {
    if (context)
        DestroyContext();
    dlclose(library);
}

NxLoader* Plugin::Load(std::string_view path) {
    const auto root_dir = new filesystem::Directory();
    const auto handle = CreateLoaderFromFile(root_dir, path);
    return new Loader(*this, handle, *root_dir);
}

u64 Plugin::GetApiVersion() { return get_api_version(); }

std::span<const u8> Plugin::Query(api::QueryType what) { return query(what); }

std::string_view Plugin::QueryString(api::QueryType what) {
    const auto buffer = Query(what);
    return std::string_view(reinterpret_cast<const char*>(buffer.data()),
                            buffer.size());
}

void Plugin::CreateContext(const std::map<std::string, std::string>& options) {
    std::vector<api::Option> options_vec;
    options_vec.reserve(options.size());
    for (const auto& [key, value] : options) {
        options_vec.emplace_back(
            api::Slice<const char>(std::string_view(key)),
            api::Slice<const char>(std::string_view(value)));
    }
    const auto ret =
        create_context(api::Slice(std::span<const api::Option>(options_vec)));
    ASSERT_THROWING(ret.res == api::CreateContextResult::Success, Loader,
                    ContextError::CreationFailed,
                    "Failed to create context ({})", ret.res);

    context = ret.value;
    ASSERT_THROWING(context, Loader, ContextError::CreationFailed,
                    "Failed to create context");
}

void Plugin::DestroyContext() { destroy_context(context); }

void* Plugin::CreateLoaderFromFile(filesystem::Directory* root_dir,
                                   std::string_view path) {
    const auto ret = create_loader_from_file(context, this, AddFile, root_dir,
                                             api::Slice(std::span(path)));
    if (ret.res != api::CreateLoaderFromFileResult::Success) {
        throw ret.res;
    }

    return ret.value;
}

void Plugin::LoaderDestroy(void* loader) { loader_destroy(loader); }

void Plugin::FileDestroy(void* file) { file_destroy(file); }

void* Plugin::FileOpen(void* file) { return file_open(file); }

u64 Plugin::FileGetSize(void* file) { return file_get_size(file); }

void Plugin::StreamDestroy(void* stream) { stream_destroy(stream); }

u64 Plugin::StreamGetSeek(void* stream) { return stream_get_seek(stream); }

void Plugin::StreamSeekTo(void* stream, u64 offset) {
    stream_seek_to(stream, offset);
}

void Plugin::StreamSeekBy(void* stream, u64 offset) {
    stream_seek_by(stream, offset);
}

u64 Plugin::StreamGetSize(void* stream) { return stream_get_size(stream); }

void Plugin::StreamReadRaw(void* stream, std::span<u8> buffer) {
    stream_read_raw(stream, api::Slice(buffer));
}

} // namespace hydra::horizon::loader::plugins
