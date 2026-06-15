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

struct ParcelFlattenedBinder {
    u32 type;
    u32 flags;
    i64 binder_id;
    i64 cookie;
    u64 service_name;
    u64 _unknown_x20;
};

class ParcelReader {
  public:
    ParcelReader(io::MemoryStream stream_) : stream{stream_} {
        auto header = Read<ParcelHeader>();
        stream.SeekTo(header.data_offset);
    }

    template <typename T>
    const std::span<const T> ReadSpan(usize count) {
        const auto span = stream.ReadSpan<T>(count);

        // Align
        usize size = count * sizeof(T);
        stream.SeekBy(align(size, static_cast<usize>(4)) - size);

        return span;
    }

    template <typename T>
    const T* ReadPtr() {
        return ReadSpan<T>(1).data();
    }

    template <typename T>
    T Read() {
        return *ReadPtr<T>();
    }

    template <typename T>
    const T* ReadFlattenedObject() {
        auto len = Read<i32>();      // len
        auto fd_count = Read<i32>(); // fd count

        ASSERT_DEBUG(len == sizeof(T), Services,
                     "Invalid flattened object length {}", len);
        ASSERT_DEBUG(fd_count == 0, Services,
                     "Non-zero FD count ({}) not supported", fd_count);

        return ReadPtr<T>();
    }

    template <typename T>
    const T* ReadStrongPointer() {
        bool is_valid = Read<bool>();
        if (is_valid)
            return ReadFlattenedObject<T>();
        else
            return nullptr;
    }

    // TODO: check this
    std::string ReadString16() {
        usize length = static_cast<usize>(Read<i32>());
        auto data = ReadSpan<u16>(length + 1);

        std::string str(length, '\0');
        for (usize i = 0; i < length + 1; i++)
            str[i] = static_cast<char>(data[i]);

        return str;
    }

    std::string ReadInterfaceToken() {
        const auto unknown = Read<i32>();
        ASSERT_DEBUG(unknown == 0x100, Services,
                     "Invalid interface token unknown 0x{:x}", unknown);

        return ReadString16();
    }

  private:
    io::MemoryStream stream;
};

class ParcelWriter {
  public:
    ParcelWriter(io::MemoryStream stream_) : stream{stream_} {
        header = stream.WriteReturningPtr<ParcelHeader>({
            .data_size = 0x0,
            .data_offset = sizeof(ParcelHeader),
            .objects_size = 0x0,
            .objects_offset = 0x0,
        });
    }

    void Finish() {
        header->data_size =
            static_cast<u32>(stream.GetSeek() - header->data_offset);
        header->objects_size = static_cast<u32>(objects.size() * sizeof(u32));
        header->objects_offset = header->data_offset + header->data_size;
        stream.SeekTo(header->objects_offset);
        stream.WriteSpan(std::span<const u32>(objects));
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
        auto span = stream.WriteReturningSpan<T>(count);

        // Align
        usize size = count * sizeof(T);
        stream.SeekBy(align(size, static_cast<usize>(4)) - size);

        return span;
    }

    template <typename T>
    void Write(const T& value) {
        WriteReturningPtr(value);
    }

    template <typename T>
    void WriteFlattenedObject(const T& object) {
        Write<i32>(sizeof(T)); // len
        Write<i32>(0);         // FD count
        Write(object);
    }

    template <typename T>
    void WriteStrongPointer(const T* ptr) {
        Write(ptr != nullptr);
        if (ptr)
            WriteFlattenedObject(*ptr);
    }

    // TODO: take the object instead of binder ID
    void WriteObject(u32 binder_id, u64 service_name) {
        Write<ParcelFlattenedBinder>({
            .type = 0x2,
            .flags = 0x0,
            .binder_id = binder_id,
            .cookie = 0x0,
            .service_name = service_name,
            ._unknown_x20 = 0x0,
        });

        // TODO: what is this?
        objects.push_back(0x0);
    }

    // TODO: check this
    void WriteString16(const std::string_view str) {
        ASSERT_DEBUG(str.size() != 0, Services, "Invalid string size");
        Write(static_cast<i32>(str.size()));
        auto span = WriteReturningSpan<u16>(str.size() + 1);

        for (u32 i = 0; i < str.size(); i++)
            span[i] = static_cast<u16>(str[i]);
        span[str.size()] = u'\0';
    }

    void WriteInterfaceToken(const std::string_view token) {
        Write<i32>(0x100);
        WriteString16(token);
    }

    usize GetWrittenSize() const {
        return sizeof(ParcelHeader) + header->data_size + header->objects_size;
    }

  private:
    io::MemoryStream stream;

    ParcelHeader* header;
    std::vector<u32> objects;
};

} // namespace hydra::horizon::services::hosbinder
