#pragma once

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/file.hpp"

namespace hydra::horizon::loader::plugins::api {

enum class Function {
    GetApiVersion,
    CreateContext,
    DestroyContext,
    Query,
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
};

typedef u32 (*GetApiVersionFnT)();

enum class CreateContextResult : u32 {
    Success = 0,
    AllocationFailed = 1,
    InvalidOption = 2,
    DuplicateOption = 3,
};

struct ContextOption {
    Slice<const char> key;
    Slice<const char> value;
};

typedef ReturnValue<CreateContextResult, void*> (*CreateContextFnT)(
    Slice<ContextOption>);

typedef u32 (*DestroyContextFnT)(void*);

enum class QueryType : u32 {
    Name = 0,
    DisplayVersion = 1,
    SupportedFormats = 2,
};

enum class QueryResult : u32 {
    Success = 0,
    BufferTooSmall = 1,
};

typedef ReturnValue<QueryResult, u64> (*QueryFnT)(void*, QueryType, Slice<u8>);

typedef void (*add_file)(void*, filesystem::Directory*, Slice<const char>,
                         void*);

enum class CreateLoaderFromFileResult : u32 {
    Success = 0,
    AllocationFailed = 1,
    UnsupportedFile = 2,
};

typedef ReturnValue<CreateLoaderFromFileResult, void*> (
    *CreateLoaderFromFileFnT)(void*, void*, add_file, void*, Slice<const char>);

typedef void (*LoaderDestroyFnT)(void*);

typedef void (*FileDestroyFnT)(void*);
typedef void* (*FileOpenFnT)(void*);
typedef u64 (*FileGetSizeFnT)(void*);

typedef void (*StreamDestroyFnT)(void*);
typedef u64 (*StreamGetSeekFnT)(void*);
typedef void (*StreamSeekToFnT)(void*, u64);
typedef void (*StreamSeekByFnT)(void*, u64);
typedef u64 (*StreamGetSizeFnT)(void*);
typedef void (*StreamReadRawFnT)(void*, Slice<u8>);

} // namespace hydra::horizon::loader::plugins::api

ENABLE_ENUM_FORMATTING(
    hydra::horizon::loader::plugins::api::CreateContextResult, Success,
    "success", AllocationFailed, "allocation failed", InvalidOption,
    "invalid option", DuplicateOption, "duplicate option")

ENABLE_ENUM_FORMATTING(hydra::horizon::loader::plugins::api::QueryResult,
                       Success, "success", BufferTooSmall, "buffer too small")

ENABLE_ENUM_FORMATTING(
    hydra::horizon::loader::plugins::api::CreateLoaderFromFileResult, Success,
    "success", AllocationFailed, "allocation failed", UnsupportedFile,
    "unsupported file")
