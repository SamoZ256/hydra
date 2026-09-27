#include "core/horizon/filesystem/patch/ips.hpp"

namespace hydra::horizon::filesystem::patch {

namespace {

constexpr std::string_view IPS_MAGIC = "PATCH";
constexpr std::string_view IPS_TAIL = "EOF";
constexpr std::string_view IPS32_MAGIC = "IPS32";
constexpr std::string_view IPS32_TAIL = "EEOF";

// The offset field and the tail marker are the same width in both variants
class Reader {
  public:
    Reader(std::span<const u8> bytes_, const std::string_view tail_)
        : bytes{bytes_}, tail{tail_}, offset{IPS_MAGIC.size()} {}

    bool read(std::span<u8> out) {
        if (offset + out.size() > bytes.size())
            return false;

        std::memcpy(out.data(), bytes.data() + offset, out.size());
        offset += out.size();
        return true;
    }

    bool readBigEndian(const usize size, u32& out_value) {
        std::array<u8, sizeof(u32)> buffer{};
        if (!read(std::span(buffer).first(size)))
            return false;

        out_value = 0;
        for (usize i = 0; i < size; i++)
            out_value = (out_value << 8) | buffer[i];

        return true;
    }

    usize getTailSize() const { return tail.size(); }

    bool isTail(std::span<const u8> buffer) const {
        return std::equal(tail.begin(), tail.end(), buffer.begin());
    }

  private:
    std::span<const u8> bytes;
    std::string_view tail;
    usize offset;
};

} // namespace

std::optional<Patch> parseIps(std::span<const u8> bytes,
                              const BuildId& build_id,
                              const std::string_view name) {
    if (bytes.size() < IPS_MAGIC.size())
        return std::nullopt;

    const std::string_view magic(reinterpret_cast<const char*>(bytes.data()),
                                 IPS_MAGIC.size());
    std::string_view tail;
    if (magic == IPS32_MAGIC) {
        tail = IPS32_TAIL;
    } else if (magic == IPS_MAGIC) {
        tail = IPS_TAIL;
    } else {
        LOG_ERROR(Loader, "Invalid IPS magic in \"{}\"", name);
        return std::nullopt;
    }

    Reader reader(bytes, tail);
    std::vector<Record> records;
    while (true) {
        std::array<u8, sizeof(u32)> buffer{};
        const auto head = std::span(buffer).first(reader.getTailSize());
        if (!reader.read(head)) {
            LOG_ERROR(Loader, "Unexpected end of \"{}\"", name);
            return std::nullopt;
        }

        if (reader.isTail(head))
            break;

        u32 offset = 0;
        for (const auto byte : head)
            offset = (offset << 8) | byte;

        u32 size = 0;
        if (!reader.readBigEndian(2, size)) {
            LOG_ERROR(Loader, "Unexpected end of \"{}\"", name);
            return std::nullopt;
        }

        std::vector<u8> data;
        if (size == 0) {
            // Run-length encoded record
            u32 rle_size = 0;
            std::array<u8, 1> value{};
            if (!reader.readBigEndian(2, rle_size) || !reader.read(value)) {
                LOG_ERROR(Loader, "Unexpected end of \"{}\"", name);
                return std::nullopt;
            }

            data.assign(rle_size, value[0]);
        } else {
            data.resize(size);
            if (!reader.read(data)) {
                LOG_ERROR(Loader, "Unexpected end of \"{}\"", name);
                return std::nullopt;
            }
        }

        records.push_back({.offset = offset, .data = std::move(data)});
    }

    if (records.empty()) {
        LOG_WARN(Loader, "Patch \"{}\" contains no records", name);
        return std::nullopt;
    }

    std::vector<Block> blocks;
    blocks.push_back({.name = std::string(name),
                      .enabled = true,
                      .records = std::move(records)});

    return Patch(build_id, name, std::move(blocks));
}

} // namespace hydra::horizon::filesystem::patch
