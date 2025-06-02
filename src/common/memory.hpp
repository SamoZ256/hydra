#pragma once

#include <istream>
#include <ostream>

#include "common/types.hpp"

namespace hydra {

class Reader {
  public:
    Reader(const u8* base_, usize size_)
        : base{base_}, ptr{base_}, size{size_} {}

    Reader CreateSubReader(usize new_size = invalid<usize>()) {
        return Reader(ptr, std::min<usize>(new_size, size - Tell()));
    }

    u64 Tell() const { return static_cast<u64>(ptr - base); }
    void Seek(u64 pos) { ptr = base + pos; }
    void Skip(usize size) { ptr += size; }

    template <typename T> const T* ReadPtr(usize count = 1) {
        const T* result = reinterpret_cast<const T*>(ptr);
        ptr += sizeof(T) * count;

        return result;
    }

    template <typename T> const T Read() { return *ReadPtr<T>(); }

    std::string ReadString() {
        std::string result(reinterpret_cast<const char*>(ptr));
        ptr += result.size();

        return result;
    }

    // Getters
    const u8* GetBase() const { return base; }
    const u8* GetPtr() const { return ptr; }
    usize GetSize() const { return size; }
    usize GetReadSize() const { return ptr - base; }

    bool IsValid() const { return base != nullptr; }

  private:
    const u8* base;
    const u8* ptr;
    usize size;
};

class Writer {
  public:
    Writer(u8* base_, usize size_) : base{base_}, ptr{base_}, size{size_} {}

    u64 Tell() { return static_cast<u64>(ptr - base); }
    void Seek(u64 pos) { ptr = base + pos; }
    void Skip(usize size) { ptr += size; }

    template <typename T> T* Write(const T& value) {
        T* result = reinterpret_cast<T*>(ptr);
        *result = value;
        ptr += sizeof(T);

        return result;
    }

    template <typename T>
    T* WritePtr(const T* data = nullptr, usize count = 1) {
        T* result = reinterpret_cast<T*>(ptr);
        usize s = sizeof(T) * count;
        if (data)
            memcpy(result, data, s);
        ptr += s;

        return result;
    }

    void WriteString(const std::string_view str) {
        char* result = reinterpret_cast<char*>(ptr);
        memcpy(result, str.data(), str.size());
        result[str.size()] = '\0';
        ptr += str.size() + 1;
    }

    // Getters
    u8* GetBase() const { return base; }
    usize GetSize() const { return size; }
    usize GetWrittenSize() const { return ptr - base; }

    bool IsValid() const { return base != nullptr; }

  private:
    u8* base;
    u8* ptr;
    usize size;
};

class StreamReader {
  public:
    StreamReader(std::istream& stream_, u64 offset_ = 0,
                 usize size_ = invalid<usize>())
        : stream{stream_}, offset{offset_}, size{size_} {
        if (size == invalid<usize>()) {
            stream.seekg(0, std::ios::end);
            size = Tell();
        }
        Seek(0);
    }

    StreamReader CreateSubReader(usize new_size = invalid<usize>()) {
        return StreamReader(stream, stream.tellg(),
                            std::min<usize>(new_size, size - Tell()));
    }

    u64 Tell() const { return static_cast<u64>(stream.tellg()) - offset; }
    void Seek(u64 pos) { stream.seekg(offset + pos, std::ios::beg); }

    template <typename T> T Read() {
        T result;
        stream.read(reinterpret_cast<char*>(&result), sizeof(T));

        return result;
    }

    template <typename T> void ReadPtr(T* ptr, usize count) {
        stream.read(reinterpret_cast<char*>(ptr), count * sizeof(T));
    }

    // Getters
    u64 GetOffset() const { return offset; }

    usize GetSize() const { return size; }

  private:
    std::istream& stream;
    u64 offset;
    usize size;
};

class StreamWriter {
  public:
    StreamWriter(std::ostream& stream_, u64 offset_ = 0,
                 usize size_ = invalid<usize>())
        : stream{stream_}, offset{offset_}, size{size_} {
        if (size == invalid<usize>()) {
            stream.seekp(0, std::ios::end);
            size = Tell();
        }
        Seek(0);
    }

    u64 Tell() { return static_cast<u64>(stream.tellp()) - offset; }
    void Seek(u64 pos) { stream.seekp(offset + pos, std::ios::beg); }

    template <typename T> void Write(const T& value) {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template <typename T> void WritePtr(const T* write_ptr, usize count) {
        stream.write(reinterpret_cast<const char*>(write_ptr),
                     count * sizeof(T));
    }

    // Getters
    u64 GetOffset() const { return offset; }

    usize GetSize() const { return size; }

  private:
    std::ostream& stream;
    u64 offset;
    usize size;
};

} // namespace hydra
