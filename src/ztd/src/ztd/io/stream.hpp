#pragma once

#include "ztd/builtin.hpp"
#include "ztd/type_aliases.hpp"

namespace ztd::io {

class IStream {
    friend class StreamView;
    friend class SparseStream;

  public:
    IStream() noexcept = default;
    virtual ~IStream() noexcept = default;

    ZTD_MAKE_DEFAULT_COPYABLE(IStream);
    ZTD_MAKE_DEFAULT_MOVABLE(IStream);

    [[nodiscard]] virtual auto getSeek() const noexcept -> u64 = 0;
    virtual void seekTo(u64 seek) {
        (void)seek;
        // TODO: error
        builtin::unreachable();
    }
    virtual auto seekBy(u64 offset) noexcept -> void = 0;

    [[nodiscard]] virtual auto getSize() const noexcept -> u64 = 0;
    [[nodiscard]] auto getRemainingSize() const noexcept -> u64 {
        return getSize() - getSeek();
    }

    virtual auto flush() noexcept -> void {}

    // Read
    template <typename T>
    [[nodiscard]] auto read() noexcept -> T {
        T result;
        readRaw(std::span(reinterpret_cast<u8*>(&result), sizeof(T)));
        return result;
    }

    template <typename T>
    auto readToRef(T& result) noexcept -> void {
        readRaw(std::span(reinterpret_cast<u8*>(&result), sizeof(T)));
    }

    template <typename T>
    auto readToSpan(std::span<T> buffer) noexcept -> void {
        readRaw(std::span(reinterpret_cast<u8*>(buffer.data()),
                          buffer.size_bytes()));
    }

    template <typename T>
    [[nodiscard]] auto readPtr() noexcept -> const T* {
        return reinterpret_cast<const T*>(consumePtrRaw(sizeof(T)));
    }

    template <typename T>
    [[nodiscard]] auto readSpan(usize count = 1) noexcept
        -> std::span<const T> {
        const auto ptr =
            reinterpret_cast<const T*>(consumePtrRaw(count * sizeof(T)));
        return std::span<const T>(ptr, count);
    }

    template <typename T>
    [[nodiscard]] auto readSpanWhole() noexcept -> std::span<const T> {
        return readSpan<T>((getSize() - getSeek()) / sizeof(T));
    }

    [[nodiscard]] auto readString(usize size) noexcept -> std::string_view {
        const auto* const ptr = readPtr<char>();
        return {ptr, size};
    }

    [[nodiscard]] auto readNullTerminatedString() noexcept -> std::string_view {
        const char* ptr = readPtr<char>();
        usize size = 0;
        while (ptr[size] != '\0') {
            seekBy(1);
            size++;
        }

        return {ptr, size};
    }

    // Write
    template <typename T>
    auto write(const T& value) noexcept -> void {
        writeRaw(std::span(reinterpret_cast<const u8*>(&value), sizeof(T)));
    }

    template <typename T>
    auto writeSpan(std::span<const T> span) noexcept -> void {
        writeRaw(std::span(reinterpret_cast<const u8*>(span.data()),
                           span.size_bytes()));
    }

    template <typename T>
    [[nodiscard]] auto writeReturningPtr() noexcept -> T* {
        return writeReturningSpan<T>(1).data();
    }

    template <typename T>
    [[nodiscard]] auto writeReturningPtr(const T& value) noexcept -> T* {
        auto ptr = writeReturningPtr<T>();
        *ptr = value;
        return ptr;
    }

    template <typename T>
    [[nodiscard]] auto writeReturningSpan(usize count) noexcept
        -> std::span<T> {
        return std::span<T>(
            reinterpret_cast<T*>(consumePtrRaw(count * sizeof(T))), count);
    }

    auto writeNullTerminatedString(std::string_view str) noexcept -> void {
        writeSpan(
            std::span(reinterpret_cast<const u8*>(str.data()), str.size()));
        write('\0');
    }

  protected:
    // TODO: verify size
    virtual auto readRaw(std::span<u8> buffer) noexcept -> void {
        (void)buffer;
        // TODO: error
        builtin::unreachable();
    }

    // TODO: verify size
    virtual auto writeRaw(std::span<const u8> buffer) noexcept -> void {
        (void)buffer;
        // TODO: error
        builtin::unreachable();
    }

    // TODO: verify size
    virtual auto consumePtrRaw(usize size) noexcept -> u8* {
        (void)size;
        // TODO: error
        builtin::unreachable();
    }
};

} // namespace ztd::io
