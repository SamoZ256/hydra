#include "core/horizon/filesystem/patch/patch.hpp"

#include "core/horizon/filesystem/patch/ips.hpp"
#include "core/horizon/filesystem/patch/pchtxt.hpp"

namespace hydra::horizon::filesystem::patch {

namespace {

constexpr std::string_view HEX_DIGITS = "0123456789abcdef";

std::optional<std::vector<u8>> readFile(const std::filesystem::path& path) {
    std::ifstream ifs(path, std::ios::binary | std::ios::ate);
    if (!ifs)
        return std::nullopt;

    const auto size = ifs.tellg();
    if (size < 0)
        return std::nullopt;

    std::vector<u8> bytes(static_cast<usize>(size));
    ifs.seekg(0);
    ifs.read(reinterpret_cast<char*>(bytes.data()), size);
    if (!ifs)
        return std::nullopt;

    return bytes;
}

} // namespace

std::string buildIdToString(const BuildId& build_id) {
    std::string str;
    str.reserve(build_id.size() * 2);
    for (const auto byte : build_id) {
        str.push_back(HEX_DIGITS[byte >> 4]);
        str.push_back(HEX_DIGITS[byte & 0xf]);
    }

    return str;
}

std::optional<BuildId> buildIdFromString(std::string_view str) {
    // Trailing zeros can be left out
    std::string padded(str);
    if (padded.size() > BUILD_ID_SIZE * 2)
        return std::nullopt;
    padded.resize(BUILD_ID_SIZE * 2, '0');

    BuildId build_id{};
    for (usize i = 0; i < build_id.size(); i++) {
        const auto byte =
            fromChars<u8>(std::string_view(padded).substr(i * 2, 2), 16);
        if (!byte)
            return std::nullopt;

        build_id[i] = *byte;
    }

    return build_id;
}

void Patch::apply(std::span<u8> image) const {
    usize applied = 0;
    for (const auto& block : blocks) {
        if (!block.enabled)
            continue;

        for (const auto& record : block.records) {
            usize offset = record.offset;
            usize data_offset = 0;
            usize size = record.data.size();

            // The NSO header is not part of the mapped image
            if (offset < NSO_HEADER_SIZE) {
                const usize diff = NSO_HEADER_SIZE - offset;
                if (diff >= size)
                    continue;

                offset += diff;
                data_offset += diff;
                size -= diff;
            }

            const usize image_offset = offset - NSO_HEADER_SIZE;
            if (image_offset >= image.size()) {
                LOG_WARN(Loader,
                         "Patch record at {:#010x} is outside of the image",
                         record.offset);
                continue;
            }

            size = std::min(size, image.size() - image_offset);
            std::memcpy(image.data() + image_offset,
                        record.data.data() + data_offset, size);
            applied++;
        }
    }

    LOG_INFO(Loader, "Applied {} records from \"{}\"", applied, name);
}

void Collection::scanFile(const std::filesystem::path& path) {
    const auto extension = toLower(path.extension().string());
    std::optional<Patch> patch;
    if (extension == PCHTXT_EXTENSION) {
        const auto bytes = readFile(path);
        if (!bytes) {
            LOG_ERROR(Loader, "Failed to read \"{}\"", path.string());
            return;
        }

        patch = parsePchtxt(
            std::string_view(reinterpret_cast<const char*>(bytes->data()),
                             bytes->size()),
            path.filename().string());
    } else if (extension == IPS_EXTENSION) {
        // IPS carries no build ID, use the filename
        const auto build_id = buildIdFromString(path.stem().string());
        if (!build_id) {
            LOG_ERROR(Loader, "\"{}\" is not named after a build ID",
                      path.string());
            return;
        }

        const auto bytes = readFile(path);
        if (!bytes) {
            LOG_ERROR(Loader, "Failed to read \"{}\"", path.string());
            return;
        }

        patch = parseIps(*bytes, *build_id, path.filename().string());
    } else {
        return;
    }

    if (!patch)
        return;

    LOG_INFO(Loader, "Found patch \"{}\" for build id {}", patch->getName(),
             buildIdToString(patch->getBuildId()));
    patches.push_back(std::move(*patch));
}

void Collection::scan(const std::vector<std::string>& paths) {
    patches.clear();
    for (const auto& path : paths) {
        std::error_code error;
        if (!std::filesystem::exists(path, error)) {
            LOG_ERROR(Loader, "Patch path does not exist: \"{}\"", path);
            continue;
        }

        if (!std::filesystem::is_directory(path, error)) {
            scanFile(path);
            continue;
        }

        for (const auto& entry :
             std::filesystem::recursive_directory_iterator(path, error)) {
            if (entry.is_regular_file(error))
                scanFile(entry.path());
        }
    }
}

void Collection::apply(const BuildId& build_id, std::span<u8> image) const {
    for (const auto& patch : patches) {
        if (buildIdsMatch(patch.getBuildId(), build_id))
            patch.apply(image);
    }
}

} // namespace hydra::horizon::filesystem::patch
