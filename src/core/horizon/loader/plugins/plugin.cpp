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

    usize GetSize() override { return plugin.FileGetSize(handle); }

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

Plugin::Plugin(const std::string& path,
               const std::map<std::string, std::string>& options) {
    library = dlopen(path.data(), RTLD_LAZY);
    ASSERT_THROWING(library, Loader, Error::LoadFailed,
                    "Failed to load plugin at path {}: {}", path, dlerror());

    // Functions
    get_api_version =
        LoadFunction<api::Function::GetApiVersion, api::GetApiVersionFnT>();
    create_context =
        LoadFunction<api::Function::CreateContext, api::CreateContextFnT>();
    destroy_context =
        LoadFunction<api::Function::DestroyContext, api::DestroyContextFnT>();
    query = LoadFunction<api::Function::Query, api::QueryFnT>();
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

    // Context
    context = CreateContext(options);
    ASSERT_THROWING(context, Loader, Error::ContextCreationFailed,
                    "Failed to create context");

    // Info
    std::array<u8, 1024> buffer;
    name = Query(api::QueryType::Name, buffer);
    display_version = Query(api::QueryType::DisplayVersion, buffer);
    supported_formats = split<std::string>(
        Query(api::QueryType::SupportedFormats, buffer), ',');

    LOG_INFO(Loader,
             "Loaded plugin \"{}\" (version: {}, formats: {}) at path \"{}\"",
             name, display_version, supported_formats, path);
}

Plugin::~Plugin() {
    DestroyContext();
    dlclose(library);
}

NxLoader* Plugin::Load(std::string_view path) {
    const auto root_dir = new filesystem::Directory();
    const auto handle = CreateLoaderFromFile(root_dir, path);
    return new Loader(*this, handle, *root_dir);
}

u64 Plugin::GetApiVersion() { return get_api_version(); }

void* Plugin::CreateContext(const std::map<std::string, std::string>& options) {
    std::vector<api::ContextOption> options_vec;
    options_vec.reserve(options.size());
    for (const auto& [key, value] : options) {
        options_vec.emplace_back(
            api::Slice<const char>(std::string_view(key)),
            api::Slice<const char>(std::string_view(value)));
    }
    const auto ret = create_context(api::Slice(std::span(options_vec)));
    if (ret.res != api::CreateContextResult::Success) {
        throw ret.res;
    }

    return ret.value;
}

void Plugin::DestroyContext() { destroy_context(context); }

std::string Plugin::Query(api::QueryType what, std::span<u8> buffer) {
    const auto ret = query(context, what, api::Slice(buffer));
    switch (ret.res) {
    case api::QueryResult::Success:
        break;
    case api::QueryResult::BufferTooSmall:
        LOG_FATAL(Loader, "Buffer too small");
    default:
        LOG_FATAL(Loader, "Unknown query result: {}", ret.res);
    }

    return std::string(buffer.begin(),
                       buffer.begin() + static_cast<i32>(ret.value));
}

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
