#pragma once

#include <istream>
#include <ostream>

#include "common/types.hpp"

namespace Hydra {

class Reader {
  public:
    Reader(u8* base_, usize size_) : base{base_}, ptr{base_}, size{size_} {}

    u64 Tell() { return static_cast<u64>(ptr - base); }

    void Seek(u64 pos) { ptr = base + pos; }

    void Skip(usize size) { ptr += size; }

    template <typename T> T* ReadPtr(usize count = 1) {
        T* result = reinterpret_cast<T*>(ptr);
        ptr += sizeof(T) * count;

        return result;
    }

    template <typename T> T Read() { return *ReadPtr<T>(); }

    std::string ReadString() {
        std::string result(reinterpret_cast<char*>(ptr));
        ptr += result.size();

        return result;
    }

    // Getters
    u8* GetBase() const { return base; }
    usize GetSize() const { return size; }
    usize GetReadSize() const { return ptr - base; }

  private:
    u8* base;
    u8* ptr;
    usize size;
};

class StreamReader {
  public:
    StreamReader(std::istream& stream_, u64 offset_, usize size_)
        : stream{stream_}, offset{offset_}, size{size_} {
        Seek(0);
    }

    StreamReader CreateSubReader(usize new_size) {
        return StreamReader(stream, stream.tellg(), new_size);
    }

    StreamReader CreateSubReader() { return CreateSubReader(size - Tell()); }

    u64 Tell() { return static_cast<u64>(stream.tellg()) - offset; }

    void Seek(u64 pos) { stream.seekg(offset + pos, std::ios::beg); }

    template <typename T> T Read() {
        T result;
        stream.read(reinterpret_cast<char*>(&result), sizeof(T));

        return result;
    }

    template <typename T> void Read(T* ptr, usize count) {
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

class Writer {
  public:
    Writer(u8* base_, usize size_) : base{base_}, ptr{base_}, size{size_} {}

    template <typename T> T* Write(const T& value) {
        T* result = reinterpret_cast<T*>(ptr);
        *result = value;
        ptr += sizeof(T);

        return result;
    }

    template <typename T> T* Write(const T* write_ptr, usize count) {
        T* result = reinterpret_cast<T*>(ptr);
        memcpy(result, write_ptr, sizeof(T) * count);
        ptr += sizeof(T) * count;

        return result;
    }

    // Getters
    u8* GetBase() const { return base; }
    usize GetSize() const { return size; }
    usize GetWrittenSize() const { return ptr - base; }

  private:
    u8* base;
    u8* ptr;
    usize size;
};

class StreamWriter {
  public:
    StreamWriter(std::ostream& stream_, u64 offset_, usize size_)
        : stream{stream_}, offset{offset_}, size{size_} {
        Seek(0);
    }

    u64 Tell() { return static_cast<u64>(stream.tellp()) - offset; }

    void Seek(u64 pos) { stream.seekp(offset + pos, std::ios::beg); }

    template <typename T> void Write(const T& value) {
        stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
    }

    template <typename T> void Write(const T* write_ptr, usize count) {
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

} // namespace Hydra
