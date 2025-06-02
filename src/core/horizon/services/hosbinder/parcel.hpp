#pragma once

namespace hydra::horizon::services::hosbinder {

struct ParcelHeader {
    u32 data_size;
    u32 data_offset;
    u32 objects_size;
    u32 objects_offset;
};

struct ParcelFlattenedObject {
    i32 size;
    i32 fd_count;
};

class ParcelReader {
  public:
    ParcelReader(Reader& reader_) : reader{reader_} {
        auto header = Read<ParcelHeader>();
        reader.Seek(header.data_offset);
    }

    template <typename T> const T* ReadPtr(usize count = 1) {
        const T* ptr = reader.ReadPtr<T>(count);

        // Align
        usize size = sizeof(T) * count;
        reader.Skip(align(size, (usize)4) - size);

        return ptr;
    }

    template <typename T> T Read() { return *ReadPtr<T>(); }

    template <typename T> const T* ReadFlattenedObject() {
        auto len = Read<i32>();      // len
        auto fd_count = Read<i32>(); // fd count

        ASSERT_DEBUG(len == sizeof(T), Services,
                     "Invalid flattened object length {}", len);
        ASSERT_DEBUG(fd_count == 0, Services,
                     "Non-zero FD count ({}) not supported", fd_count);

        return ReadPtr<T>();
    }

    template <typename T> const T* ReadStrongPointer() {
        bool is_valid = Read<bool>();
        if (is_valid)
            return ReadFlattenedObject<T>();
        else
            return nullptr;
    }

    std::string ReadString16() {
        usize length = Read<i32>();
        auto data = ReadPtr<u16>(length + 1);

        std::string str(length, '\0');
        for (usize i = 0; i < length + 1; i++)
            str[i] = data[i];

        return str;
    }

    std::string ReadInterfaceToken() {
        const auto unknown = Read<i32>();
        ASSERT_DEBUG(unknown == 0x100, Services,
                     "Invalid interface token unknown 0x{:x}", unknown);

        return ReadString16();
    }

  private:
    Reader& reader;
};

class ParcelWriter {
  public:
    ParcelWriter(Writer& writer_) : writer{writer_} {
        header = writer.WritePtr<ParcelHeader>();
        header->data_size = 0;
        header->data_offset = sizeof(ParcelHeader);
        header->objects_size = 0;
        header->objects_offset = 0;
    }

    void Finalize() { header->data_size = writer.Tell() - header->data_offset; }

    template <typename T>
    T* WritePtr(const T* data = nullptr, usize count = 1) {
        auto ptr = writer.WritePtr(data, count);

        // Align
        usize size = sizeof(T) * count;
        writer.Skip(align(size, (usize)4) - size);

        return ptr;
    }

    template <typename T> void Write(const T& value) { WritePtr(&value); }

    template <typename T> void WriteFlattenedObject(const T& object) {
        Write<i32>(sizeof(T)); // len
        Write<i32>(0);         // FD count
        Write(object);
    }

    template <typename T> void WriteStrongPointer(const T* ptr) {
        Write(ptr != nullptr);
        if (ptr)
            WriteFlattenedObject(*ptr);
    }

    void WriteString16(const std::string_view str) {
        ASSERT_DEBUG(str.size() != 0, Services, "Invalid string size");
        Write<i32>(str.size());
        auto ptr = WritePtr<u16>(nullptr, str.size() + 1);

        for (u32 i = 0; i < str.size() + 1; i++)
            ptr[i] = str[i];
    }

    void WriteInterfaceToken(const std::string_view token) {
        Write<i32>(0x100);
        WriteString16(token);
    }

  private:
    Writer& writer;

    ParcelHeader* header;
};

} // namespace hydra::horizon::services::hosbinder
