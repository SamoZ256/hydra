#pragma once

#include <dlfcn.h>

#include "core/horizon/loader/plugins/api.hpp"

namespace hydra::horizon::filesystem {
class Directory;
}

namespace hydra::horizon::loader {
class NxLoader;
}

namespace hydra::horizon::loader::plugins {

class Plugin {
    friend class Manager;

  public:
    enum class Error {
        LoadFailed,
        InvalidApiVersion,
        ContextCreationFailed,
    };

    Plugin(const std::string& path,
           const std::map<std::string, std::string>& options);
    ~Plugin();

    NxLoader* Load(std::string_view path);

    // API
    u64 GetApiVersion();
    void* CreateContext(const std::map<std::string, std::string>& options);
    void DestroyContext();
    std::string Query(api::QueryType what, std::span<u8> buffer);
    void* CreateLoaderFromFile(filesystem::Directory* root_dir,
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
    void* library;

    // Functions
    api::GetApiVersionFnT get_api_version;
    api::CreateContextFnT create_context;
    api::DestroyContextFnT destroy_context;
    api::QueryFnT query;
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

    // Context
    void* context;

    // Info
    std::string name;
    std::string display_version;
    std::vector<std::string> supported_formats;

    // Helpers
    enum class GetFunctionError {
        SymbolNotFound,
    };

    template <api::Function api_func, typename T>
    T LoadFunction() {
        std::string_view symbol_name;
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

        const auto func = dlsym(library, symbol_name.data());
        ASSERT_THROWING(func != nullptr, Loader,
                        GetFunctionError::SymbolNotFound,
                        "Failed to load symbol \"{}\"", symbol_name);

        return reinterpret_cast<T>(func);
    }
};

} // namespace hydra::horizon::loader::plugins
