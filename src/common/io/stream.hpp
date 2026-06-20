#pragma once

#include "common/log.hpp"
#include "common/types.hpp"

namespace hydra::io {

class IStream {
    friend class StreamView;
    friend class SparseStream;

  public:
    IStream() = default;
    virtual ~IStream() noexcept = default;

    MAKE_DEFAULT_COPYABLE(IStream);

    virtual u64 GetSeek() const = 0;
    virtual void SeekTo(u64 seek) {
        (void)seek;
        LOG_FATAL(Common, "Stream does not support arbitrary seeking");
    }
    virtual void SeekBy(u64 offset) = 0;

    virtual u64 GetSize() const = 0;
    u64 GetRemainingSize() const { return GetSize() - GetSeek(); }

    virtual void Flush() {}

    // Read
    template <typename T>
    T Read() {
        T result;
        ReadRaw(std::span(reinterpret_cast<u8*>(&result), sizeof(T)));
        return result;
    }

    template <typename T>
    void ReadToRef(T& result) {
        ReadRaw(std::span(reinterpret_cast<u8*>(&result), sizeof(T)));
    }

    template <typename T>
    void ReadToSpan(std::span<T> buffer) {
        ReadRaw(std::span(reinterpret_cast<u8*>(buffer.data()),
                          buffer.size_bytes()));
    }

    template <typename T>
    const T* ReadPtr() {
        return reinterpret_cast<const T*>(ConsumePtrRaw(sizeof(T)));
    }

    template <typename T>
    std::span<const T> ReadSpan(usize count = 1) {
        const auto ptr =
            reinterpret_cast<const T*>(ConsumePtrRaw(count * sizeof(T)));
        return std::span<const T>(ptr, count);
    }

    template <typename T>
    std::span<const T> ReadSpanWhole() {
        return ReadSpan<T>((GetSize() - GetSeek()) / sizeof(T));
    }

    std::string_view ReadString(usize size) {
        const auto ptr = ReadPtr<char>();
        return {ptr, size};
    }

    std::string_view ReadNullTerminatedString() {
        const char* ptr = ReadPtr<char>();
        usize size = 0;
        while (ptr[size] != '\0') {
            SeekBy(1);
            size++;
        }

        return {ptr, size};
    }

    // Write
    template <typename T>
    void Write(const T& value) {
        WriteRaw(std::span(reinterpret_cast<const u8*>(&value), sizeof(T)));
    }

    template <typename T>
    void WriteSpan(std::span<const T> span) {
        WriteRaw(std::span(reinterpret_cast<const u8*>(span.data()),
                           span.size_bytes()));
    }

    template <typename T>
    T* WriteReturningPtr() {
        return WriteReturningSpan<T>(1).data();
    }

    template <typename T>
    T* WriteReturningPtr(const T& value) {
        auto ptr = WriteReturningPtr<T>();
        *ptr = value;
        return ptr;
    }

    template <typename T>
    std::span<T> WriteReturningSpan(usize count) {
        return std::span<T>(
            reinterpret_cast<T*>(ConsumePtrRaw(count * sizeof(T))), count);
    }

    void WriteNullTerminatedString(std::string_view str) {
        WriteSpan(
            std::span(reinterpret_cast<const u8*>(str.data()), str.size()));
        Write('\0');
    }

  protected:
    virtual void ReadRaw(std::span<u8> buffer) {
        (void)buffer;
        LOG_FATAL(Common, "Stream is write-only");
    }
    virtual void WriteRaw(std::span<const u8> buffer) {
        (void)buffer;
        LOG_FATAL(Common, "Stream is read-only");
    }
    virtual u8* ConsumePtrRaw(usize size) {
        (void)size;
        LOG_FATAL(Common, "Stream is not continuous");
    }
};

} // namespace hydra::io
