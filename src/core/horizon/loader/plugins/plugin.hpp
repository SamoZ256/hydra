#pragma once

#include <dlfcn.h>
#include <expected>

#include "core/horizon/loader/plugins/api.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::filesystem {
class Directory;
}

namespace hydra::horizon::loader {
class NxLoader;
}

namespace hydra::horizon::loader::plugins {

struct OptionConfig {
    std::string_view name;
    std::string_view description;
    api::OptionType type;
    bool is_required;
    // TODO: put these into a union?
    std::vector<std::string_view> enum_value_names;
    std::vector<std::string_view> path_content_types;
};

class Plugin {
    friend class Manager;

  public:
    enum class Error {
        LoadFailed,
        UnsupportedApiVersion,
        InvalidOptions,
        ContextCreationFailed,
    };

    // HACK: need to accept const std::string& instead of std::string_view, as
    // dlopen needs a null-terminated string
    static std::expected<Plugin, Error> Create(const std::string& path);
    static std::expected<Plugin, Error>
    Create(const std::string& path,
           const std::map<std::string, std::string>& options);

    Plugin() = default;
    ~Plugin();

    ZTD_MAKE_NON_COPYABLE(Plugin);
    ZTD_MAKE_MOVABLE(Plugin, library, std::exchange(other.library, nullptr),
                     get_api_version, other.get_api_version, query, other.query,
                     create_context, other.create_context, destroy_context,
                     other.destroy_context, create_loader_from_file,
                     other.create_loader_from_file, loader_destroy,
                     other.loader_destroy, file_destroy, other.file_destroy,
                     file_open, other.file_open, file_get_size,
                     other.file_get_size, stream_destroy, other.stream_destroy,
                     stream_get_seek, other.stream_get_seek, stream_seek_to,
                     other.stream_seek_to, stream_seek_by, other.stream_seek_by,
                     stream_get_size, other.stream_get_size, stream_read_raw,
                     other.stream_read_raw, name, other.name, display_version,
                     other.display_version, supported_formats,
                     std::move(other.supported_formats), option_configs,
                     std::move(other.option_configs), context,
                     std::exchange(other.context, nullptr));

    std::optional<NxLoader*> Load(std::string_view path);

    // API
    u64 GetApiVersion();
    std::span<const u8> Query(api::QueryType what);
    std::string_view QueryString(api::QueryType what);
    std::expected<void*, Error>
    CreateContext(const std::map<std::string, std::string>& options);
    void DestroyContext();
    std::optional<void*> CreateLoaderFromFile(filesystem::Directory* root_dir,
                                              std::string_view path);
    void LoaderDestroy(void* loader);
    void FileDestroy(void* file);
    void* FileOpen(void* file);
    u64 FileGetSize(void* file);
    void StreamDestroy(void* stream);
    u64 StreamGetSeek(void* stream);
    void StreamSeekTo(void* stream, u64 offset);
    void StreamSeekBy(void* stream, u64 offset);
    u64 StreamGetSize(void* stream);
    void StreamReadRaw(void* stream, std::span<u8> buffer);

  private:
    void* library{nullptr};

    // Functions
    api::GetApiVersionFnT get_api_version;
    api::QueryFnT query;
    api::CreateContextFnT create_context;
    api::DestroyContextFnT destroy_context;
    api::CreateLoaderFromFileFnT create_loader_from_file;
    api::LoaderDestroyFnT loader_destroy;
    api::FileDestroyFnT file_destroy;
    api::FileOpenFnT file_open;
    api::FileGetSizeFnT file_get_size;
    api::StreamDestroyFnT stream_destroy;
    api::StreamGetSeekFnT stream_get_seek;
    api::StreamSeekToFnT stream_seek_to;
    api::StreamSeekByFnT stream_seek_by;
    api::StreamGetSizeFnT stream_get_size;
    api::StreamReadRawFnT stream_read_raw;

    // Info
    std::string_view name;
    std::string_view display_version;
    std::vector<std::string_view> supported_formats;
    std::vector<OptionConfig> option_configs;

    // Context
    void* context{nullptr};

    // Helpers
    template <api::Function api_func, typename T>
    T LoadFunction() {
        std::string symbol_name;
        switch (api_func) {
        case api::Function::GetApiVersion:
            symbol_name = "hydra_ext_get_api_version";
            break;
        case api::Function::CreateContext:
            symbol_name = "hydra_ext_create_context";
            break;
        case api::Function::DestroyContext:
            symbol_name = "hydra_ext_destroy_context";
            break;
        case api::Function::Query:
            symbol_name = "hydra_ext_query";
            break;
        case api::Function::CreateLoaderFromFile:
            symbol_name = "hydra_ext_create_loader_from_file";
            break;
        case api::Function::LoaderDestroy:
            symbol_name = "hydra_ext_loader_destroy";
            break;
        case api::Function::FileDestroy:
            symbol_name = "hydra_ext_file_destroy";
            break;
        case api::Function::FileOpen:
            symbol_name = "hydra_ext_file_open";
            break;
        case api::Function::FileGetSize:
            symbol_name = "hydra_ext_file_get_size";
            break;
        case api::Function::StreamDestroy:
            symbol_name = "hydra_ext_stream_destroy";
            break;
        case api::Function::StreamGetSeek:
            symbol_name = "hydra_ext_stream_get_seek";
            break;
        case api::Function::StreamSeekTo:
            symbol_name = "hydra_ext_stream_seek_to";
            break;
        case api::Function::StreamSeekBy:
            symbol_name = "hydra_ext_stream_seek_by";
            break;
        case api::Function::StreamGetSize:
            symbol_name = "hydra_ext_stream_get_size";
            break;
        case api::Function::StreamReadRaw:
            symbol_name = "hydra_ext_stream_read_raw";
            break;
        }

        const auto func = dlsym(library, symbol_name.c_str());
        ASSERT(func != nullptr, Loader, "Failed to load symbol \"{}\"",
               symbol_name);

        return reinterpret_cast<T>(func);
    }

  public:
    CONST_REF_GETTER(name, GetName);
    CONST_REF_GETTER(display_version, GetDisplayVersion);
    CONST_REF_GETTER(supported_formats, GetSupportedFormats);
    CONST_REF_GETTER(option_configs, GetOptionConfigs);
};

} // namespace hydra::horizon::loader::plugins
