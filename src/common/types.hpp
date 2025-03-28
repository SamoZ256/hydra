#pragma once

#include <cstdint>
#include <fstream>
#include <map>
#include <stdint.h>
#include <string>

#include "common/macros.hpp"

namespace Hydra {

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using i128 = __int128_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u128 = __uint128_t;
using usize = size_t;
using uptr = uintptr_t;
using f32 = float;
using f64 = double;

using paddr = uptr;
using vaddr = uptr;
using gpu_vaddr = uptr;
using HandleId = u32;

template <typename T> struct range {
  public:
    range() : base{0}, size{0} {}
    range(T base_, usize size_) : base{base_}, size{size_} {}

    // Getters
    T GetBase() const { return base; }
    usize GetSize() const { return size; }

  private:
    T base;
    usize size;
};

struct sized_ptr {
  public:
    sized_ptr() : ptr{0x0}, size{0} {}
    sized_ptr(uptr ptr_, usize size_) : ptr{ptr_}, size{size_} {}
    sized_ptr(void* ptr_, usize size_)
        : sized_ptr(reinterpret_cast<uptr>(ptr_), size_) {}

    // Getters
    uptr GetPtr() const { return ptr; }
    u8* GetPtrU8() const { return reinterpret_cast<u8*>(ptr); }
    usize GetSize() const { return size; }

  private:
    uptr ptr;
    usize size;
};

template <typename T, usize component_count> class vec {
  public:
    vec() {}
    vec(const T& value) {
        for (usize i = 0; i < component_count; i++) {
            components[i] = value;
        }
    }
    vec(const std::initializer_list<T>& values) {
        std::copy(values.begin(), values.end(), components.begin());
    }

    T& operator[](int idx) { return components[index]; }

    T operator[](int idx) const { return components[index]; }

    T& x() { return components[0]; }
    T& y() { return components[1]; }
    T& z() { return components[2]; }
    T& w() { return components[3]; }

    T x() const { return components[0]; }
    T y() const { return components[1]; }
    T z() const { return components[2]; }
    T w() const { return components[3]; }

  private:
    std::array<T, component_count> components = {0};
};

using uchar2 = vec<u8, 2>;
using ushort2 = vec<u16, 2>;
using uint2 = vec<u32, 2>;
using ulong2 = vec<u64, 2>;
using usize2 = vec<usize, 2>;
using float2 = vec<float, 2>;

using uchar3 = vec<u8, 3>;
using ushort3 = vec<u16, 3>;
using uint3 = vec<u32, 3>;
using ulong3 = vec<u64, 3>;
using usize3 = vec<usize, 3>;
using float3 = vec<float, 3>;

template <typename T> class readonly {
  public:
    readonly() {}
    readonly(const T& value_) : value{value_} {}
    void operator=(const T&) = delete;

    operator T() const { return value; }

    // Getters
    const T& Get() const { return value; }

  private:
    T value;
} __attribute__((packed));

template <typename T> class writeonly {
  public:
    writeonly() {}
    writeonly(const T& value_) : value{value_} {}
    void operator=(const T& new_value) { value = new_value; }

    // Getters
    const T& Get() const { return value; }

  private:
    T value;
} __attribute__((packed));

template <typename T> class readwrite {
  public:
    readwrite() {}
    readwrite(const T& value_) : value{value_} {}
    void operator=(const T& new_value) { value = new_value; }

    operator T() const { return value; }

    // Getters
    const T& Get() const { return value; }

  private:
    T value;
} __attribute__((packed));

class Reader {
  public:
    Reader(u8* base_) : base{base_}, ptr{base_} {}

    u64 Tell() { return static_cast<u64>(ptr - base); }

    void Seek(u64 pos) { ptr = base + pos; }

    template <typename T> T* ReadPtr() {
        T* result = reinterpret_cast<T*>(ptr);
        ptr += sizeof(T);

        return result;
    }

    template <typename T> T Read() { return *ReadPtr<T>(); }

    template <typename T> T* Read(usize count) {
        T* result = reinterpret_cast<T*>(ptr);
        ptr += sizeof(T) * count;

        return result;
    }

    std::string ReadString() {
        std::string result(reinterpret_cast<char*>(ptr));
        ptr += result.size();

        return result;
    }

    // Getters
    u8* GetBase() const { return base; }

  private:
    u8* base;
    u8* ptr;
};

class Writer {
  public:
    Writer(u8* base_) : base{base_}, ptr{base_} {}

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

    usize GetWrittenSize() const { return ptr - base; }

  private:
    u8* base;
    u8* ptr;
};

class FileReader {
  public:
    FileReader(std::ifstream& stream_, u64 offset_, usize size_)
        : stream{stream_}, offset{offset_}, size{size_} {}

    FileReader CreateSubReader(usize new_size) {
        return FileReader(stream, stream.tellg(), new_size);
    }

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
    std::ifstream& stream;
    u64 offset;
    usize size;
};

template <typename SubclassT, typename T, typename DescriptorT>
class CacheBase {
  public:
    ~CacheBase() {
        for (const auto& [key, value] : cache) {
            THIS->DestroyElement(value);
        }

        THIS->Destroy();
    }

    T Find(const DescriptorT& descriptor) {
        u64 hash = THIS->Hash(descriptor);
        auto& element = cache[hash];
        if (element) { // TODO: make this so that non-pointer types are
                       // supported
                       // as well
            THIS->Update(element);
            return element;
        }

        element = THIS->Create(descriptor);

        return element;
    }

  private:
    std::map<u64, T> cache;
};

} // namespace Hydra
