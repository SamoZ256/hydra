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
    explicit ParcelReader(ztd::io::MemoryStream stream_)
        : stream{std::move(stream_)} {
        auto header = read<ParcelHeader>();
        stream.seekTo(header.data_offset);
    }

    template <typename T>
    std::span<const T> readSpan(usize count) {
        const auto span = stream.readSpan<T>(count);

        // Align
        usize size = count * sizeof(T);
        stream.seekBy(align(size, static_cast<usize>(4)) - size);

        return span;
    }

    template <typename T>
    const T* readPtr() {
        return readSpan<T>(1).data();
    }

    template <typename T>
    T read() {
        return *readPtr<T>();
    }

    template <typename T>
    const T* readFlattenedObject() {
        auto len = read<i32>();      // len
        auto fd_count = read<i32>(); // fd count

        ASSERT_DEBUG(len == sizeof(T), Services,
                     "Invalid flattened object length {}", len);
        ASSERT_DEBUG(fd_count == 0, Services,
                     "Non-zero FD count ({}) not supported", fd_count);

        return readPtr<T>();
    }

    template <typename T>
    const T* readStrongPointer() {
        bool is_valid = read<bool>();
        if (is_valid)
            return readFlattenedObject<T>();
        else
            return nullptr;
    }

    // TODO: check this
    std::string readString16() {
        auto length = static_cast<usize>(read<i32>());
        auto data = readSpan<u16>(length + 1);

        std::string str(length, '\0');
        for (usize i = 0; i < length + 1; i++)
            str[i] = static_cast<char>(data[i]);

        return str;
    }

    std::string readInterfaceToken() {
        const auto unknown = read<i32>();
        ASSERT_DEBUG(unknown == 0x100, Services,
                     "Invalid interface token unknown 0x{:x}", unknown);

        return readString16();
    }

  private:
    ztd::io::MemoryStream stream;
};

class ParcelWriter {
  public:
    explicit ParcelWriter(ztd::io::MemoryStream stream_)
        : stream{std::move(stream_)} {
        header = stream.writeReturningPtr<ParcelHeader>({
            .data_size = 0x0,
            .data_offset = sizeof(ParcelHeader),
            .objects_size = 0x0,
            .objects_offset = 0x0,
        });
    }

    void finish() {
        header->data_size =
            static_cast<u32>(stream.getSeek() - header->data_offset);
        header->objects_size = static_cast<u32>(objects.size() * sizeof(u32));
        header->objects_offset = header->data_offset + header->data_size;
        stream.seekTo(header->objects_offset);
        stream.writeSpan(std::span<const u32>(objects));
    }

    template <typename T>
    T* writeReturningPtr() {
        return writeReturningSpan<T>(1).data();
    }

    template <typename T>
    T* writeReturningPtr(const T& value) {
        auto ptr = writeReturningPtr<T>();
        *ptr = value;
        return ptr;
    }

    template <typename T>
    std::span<T> writeReturningSpan(usize count) {
        auto span = stream.writeReturningSpan<T>(count);

        // Align
        usize size = count * sizeof(T);
        stream.seekBy(align(size, static_cast<usize>(4)) - size);

        return span;
    }

    template <typename T>
    void write(const T& value) {
        writeReturningPtr(value);
    }

    template <typename T>
    void writeFlattenedObject(const T& object) {
        write<i32>(sizeof(T)); // len
        write<i32>(0);         // FD count
        write(object);
    }

    template <typename T>
    void writeStrongPointer(const T* ptr) {
        write(ptr != nullptr);
        if (ptr)
            writeFlattenedObject(*ptr);
    }

    // TODO: take the object instead of binder ID
    void writeObject(u32 binder_id, u64 service_name) {
        write<ParcelFlattenedBinder>({
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
    void writeString16(const std::string_view str) {
        ASSERT_DEBUG(!str.empty(), Services, "Invalid string size");
        write(static_cast<i32>(str.size()));
        auto span = writeReturningSpan<u16>(str.size() + 1);

        for (u32 i = 0; i < str.size(); i++)
            span[i] = static_cast<u16>(str[i]);
        span[str.size()] = u'\0';
    }

    void writeInterfaceToken(const std::string_view token) {
        write<i32>(0x100);
        writeString16(token);
    }

    usize getWrittenSize() const {
        return sizeof(ParcelHeader) + header->data_size + header->objects_size;
    }

  private:
    ztd::io::MemoryStream stream;

    ParcelHeader* header;
    std::vector<u32> objects;
};

} // namespace hydra::horizon::services::hosbinder
