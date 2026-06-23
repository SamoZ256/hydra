#pragma once

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/file.hpp"

namespace hydra::horizon::loader::plugins::api {

enum class Function {
    GetApiVersion,
    Query,
    CreateContext,
    DestroyContext,
    CreateLoaderFromFile,
    LoaderDestroy,
    FileDestroy,
    FileOpen,
    FileGetSize,
    StreamDestroy,
    StreamGetSeek,
    StreamSeekTo,
    StreamSeekBy,
    StreamGetSize,
    StreamReadRaw,
};

template <typename Result, typename T>
struct ReturnValue {
    Result res;
    T value;
};

template <typename T>
struct Slice {
    T* data;
    u64 size;

    Slice() : data{nullptr}, size{0} {}
    Slice(T* data_, u64 size_) : data{data_}, size{size_} {}
    Slice(std::span<T> span) : data{span.data()}, size{span.size()} {}
    Slice(std::string_view str)
        requires std::is_same_v<T, char> || std::is_same_v<T, const char>
        : data{str.data()}, size{str.size()} {}

    operator std::span<T>() const { return std::span<T>(data, size); }
    operator std::string_view() const
        requires std::is_same_v<T, char> || std::is_same_v<T, const char>
    {
        return std::string_view(data, size);
    }
};

using GetApiVersionFnT = u32 (*)();

enum class QueryType : u32 {
    Name = 0,
    DisplayVersion = 1,
    SupportedFormats = 2,
    OptionConfigs = 3,
};

enum class OptionType {
    Boolean = 0,
    Integer = 1,
    Enumeration = 2,
    String = 3,
    Path = 4,
};

struct OptionConfig {
    Slice<const char> name;
    Slice<const char> description;
    OptionType type;
    bool is_required;
    union {
        Slice<const char> enum_value_names;
        Slice<const char> path_content_types;
    };
};

using QueryFnT = Slice<const u8> (*)(QueryType);

enum class CreateContextResult : u32 {
    Success = 0,
    AllocationFailed = 1,
    InvalidOption = 2,
};

struct Option {
    Slice<const char> name;
    Slice<const char> value;
};

using CreateContextFnT = ReturnValue<CreateContextResult, void*> (*)(
    Slice<const Option>);

using DestroyContextFnT = u32 (*)(void*);

using add_file = void (*)(void*, filesystem::Directory*, Slice<const char>,
                         void*);

enum class CreateLoaderFromFileResult : u32 {
    Success = 0,
    AllocationFailed = 1,
    UnsupportedFile = 2,
};

using CreateLoaderFromFileFnT = ReturnValue<CreateLoaderFromFileResult, void*> (
    *)(void*, void*, add_file, void*, Slice<const char>);

using LoaderDestroyFnT = void (*)(void*);

using FileDestroyFnT = void (*)(void*);
using FileOpenFnT = void* (*)(void*);
using FileGetSizeFnT = u64 (*)(void*);

using StreamDestroyFnT = void (*)(void*);
using StreamGetSeekFnT = u64 (*)(void*);
using StreamSeekToFnT = void (*)(void*, u64);
using StreamSeekByFnT = void (*)(void*, u64);
using StreamGetSizeFnT = u64 (*)(void*);
using StreamReadRawFnT = void (*)(void*, Slice<u8>);

} // namespace hydra::horizon::loader::plugins::api

ENABLE_ENUM_FORMATTING(
    hydra::horizon::loader::plugins::api::CreateContextResult, Success,
    "success", AllocationFailed, "allocation failed", InvalidOption,
    "invalid option")

ENABLE_ENUM_FORMATTING(
    hydra::horizon::loader::plugins::api::CreateLoaderFromFileResult, Success,
    "success", AllocationFailed, "allocation failed", UnsupportedFile,
    "unsupported file")
