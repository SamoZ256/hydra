#include "core/horizon/services/account/internal/user_manager.hpp"

#include <stb_image.h>
#include <stb_image_write.h>
#include <yaz0.h>

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/host_file.hpp"
#include "core/horizon/filesystem/romfs.hpp"

#define DEFAULT_USER_NAME "Hydra user"

namespace hydra::horizon::services::account::internal {

namespace {

constexpr magic4_t HUSR_MAGIC = make_magic4('H', 'U', 'S', 'R');
constexpr u32 CURRENT_HUSR_VERSION = 2;

struct HusrHeader {
    magic4_t magic{HUSR_MAGIC};
    u32 version{CURRENT_HUSR_VERSION};
    u32 header_size{sizeof(HusrHeader)};
};

#define DEFAULT_AVATAR_IMAGE_PATH ""
#define SYSTEM_AVATARS_PATH "$SYSTEM_AVATARS"
constexpr usize AVATAR_UNCOMPRESSED_IMAGE_SIZE = 0x40000;
constexpr u32 AVATAR_IMAGE_DIMENSIONS = 256;

static void jpg_to_memory(void* context, void* data, int len) {
    std::vector<u8>* jpg_image = static_cast<std::vector<u8>*>(context);
    u8* jpg = static_cast<u8*>(data);
    jpg_image->insert(jpg_image->end(), jpg, jpg + len);
}

} // namespace

UserManager::UserManager() {
    // Create user directory
    std::filesystem::create_directories(GetUserPath());

    // Deserialize all users
    if (std::filesystem::exists(GetUserPath())) {
        for (const auto& dir_entry :
             std::filesystem::directory_iterator{GetUserPath()}) {
            auto filename = dir_entry.path().filename();

            auto extension = filename.extension().string();
            if (extension != ".husr")
                continue;

            auto user_id_str = filename.stem().string();
            if (user_id_str.size() != 32) {
                LOG_WARN(Horizon, "Invalid user ID {}", user_id_str);
                return;
            }

            u64 user_id_hi =
                std::stoull(user_id_str.substr(0, 16), nullptr, 16);
            u64 user_id_lo =
                std::stoull(user_id_str.substr(16, 16), nullptr, 16);
            uuid_t user_id = (uuid_t)user_id_lo | ((uuid_t)user_id_hi << 64);

            Deserialize(user_id);
        }
    }

    if (users.empty()) {
        auto user_id = CreateUser();
        Serialize(user_id);
    }
}

void UserManager::Flush() {
    for (auto& [user_id, user_pair] : users)
        Serialize(user_id);
}

uuid_t UserManager::CreateUser() {
    // First, find an available ID
    uuid_t user_id = random128();
    while (user_id == 0x0 || users.contains(user_id))
        user_id = random128();

    // Create
    User new_user(DEFAULT_USER_NAME, uchar3({100, 105, 112}),
                  DEFAULT_AVATAR_IMAGE_PATH);
    users.insert({user_id, {new_user, 0}});

    return user_id;
}

void UserManager::LoadSystemAvatars(filesystem::Filesystem& fs) {
    // Default avatar
    const auto default_image_path =
        get_bundle_resource_path("default_avatar_image.png");
    avatars[DEFAULT_AVATAR_IMAGE_PATH] = {
        new filesystem::HostFile(default_image_path)};

    // NCA
    filesystem::FileBase* file;
    auto res = fs.GetFile(
        fmt::format(FS_FIRMWARE_PATH "/{:016x}/data", 0x010000000000080a),
        file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get avatars file: {}", res);
        return;
    }

    auto content_archive = new filesystem::ContentArchive(file);

    // Data
    filesystem::FileBase* data_file;
    res = content_archive->GetFile("data", data_file);
    if (res != filesystem::FsResult::Success) {
        LOG_ERROR(Services, "Failed to get avatars data: {}", res);
        return;
    }

    // RomFS
    auto romfs = new filesystem::RomFS(data_file);

    // Background
    // Not necessary

    // Characters
    filesystem::Directory* character_dir;
    res = romfs->GetDirectory("chara", character_dir);
    ASSERT(res == filesystem::FsResult::Success, Services,
           "Failed to get \"chara\" avatars directory: {}", res);
    for (const auto& [name, entry] : character_dir->GetEntries()) {
        if (name.ends_with(".szs"))
            avatars[fmt::format(SYSTEM_AVATARS_PATH "/{}", name)] = {
                static_cast<filesystem::FileBase*>(entry)};
    }
}

const std::vector<uchar4>& UserManager::LoadAvatarImage(std::string_view path,
                                                        usize& out_dimensions) {
    // Load image
    auto it = avatars.find(std::string(path));
    if (it == avatars.end()) {
        if (path[0] != '$') {
            it = avatars
                     .insert(
                         {std::string(path), {new filesystem::HostFile(path)}})
                     .first;
        } else {
            LOG_WARN(
                Services,
                "Failed to load avatar image with path {}, loading default "
                "image instead",
                path);
            it = avatars.find(DEFAULT_AVATAR_IMAGE_PATH);
            ASSERT(it != avatars.end(), Services,
                   "Failed to load default avatar image");
        }
    }
    auto& avatar = it->second;
    PreloadAvatar(avatar, path.starts_with(SYSTEM_AVATARS_PATH));

    out_dimensions = avatar.dimensions;
    return avatar.data;
}

void UserManager::LoadAvatarImageAsJpeg(std::string_view path, uchar3 bg_color,
                                        std::vector<u8>& out_data) {
    // Load image
    usize dimension;
    auto data = LoadAvatarImage(path, dimension);

    // Alpha blend with background color
    for (auto& pixel : data) {
        auto& r = pixel.x();
        auto& g = pixel.y();
        auto& b = pixel.z();
        auto& a = pixel.w();

        r = (bg_color.x() * (0xff - a) + r * a) / 0xff;
        g = (bg_color.y() * (0xff - a) + g * a) / 0xff;
        b = (bg_color.z() * (0xff - a) + b * a) / 0xff;
        a = 0xff;
    }

    // TODO: resize if needed

    // Convert to JPEG
    out_data.reserve(0x20000);
    stbi_write_jpg_to_func(jpg_to_memory, &out_data,
                           static_cast<i32>(dimension),
                           static_cast<i32>(dimension), 4, data.data(), 80);
}

void UserManager::Serialize(uuid_t user_id) {
    LOG_INFO(Horizon, "Serializing user with ID {:032x}", user_id);

    auto& user_pair = GetPair(user_id);
    const auto& user = user_pair.first;
    if (!user.EditedSince(user_pair.second))
        return;

    // Serialize
    std::ofstream ofs{fmt::format("{}/{:032x}.husr", GetUserPath(), user_id)};
    {
        StreamWriter writer(ofs);

        // Header
        HusrHeader header{};
        writer.Write(header);

        // Data
        writer.Write(user.base);
        writer.Write(user.data);
        writer.Write(user.avatar_bg_color);
        writer.Write(static_cast<u32>(user.avatar_path.size()));
        writer.WritePtr(user.avatar_path.data(), user.avatar_path.size());
    }
    ofs.close();

    user_pair.second = get_absolute_time();
}

void UserManager::Deserialize(uuid_t user_id) {
    LOG_INFO(Horizon, "Deserializing user with ID {:032x}", user_id);

    auto it = users.find(user_id);
    if (it != users.end()) {
        auto& user_pair = it->second;
        if (!user_pair.first.EditedSince(user_pair.second))
            LOG_WARN(Horizon, "Overwriting user {:032x}", user_id);
    }

    // Deserialize
    std::ifstream ifs{fmt::format("{}/{:032x}.husr", GetUserPath(), user_id)};
    StreamReader reader(ifs);

    // Header
    const auto header = reader.Read<HusrHeader>();

    // Validate
    ASSERT(header.magic == HUSR_MAGIC, Horizon,
           "Invalid HUSR magic 0x{:08x} for user {:032x}", header.magic,
           user_id);
    if (header.version < 2) {
        LOG_WARN(Horizon, "Unsupported HUSR version {} for user {:032x}",
                 header.version, user_id);
        return;
    }
    ASSERT(header.version == CURRENT_HUSR_VERSION, Horizon,
           "Invalid HUSR version {} for user {:032x}", header.version, user_id);
    ASSERT(header.header_size == sizeof(HusrHeader), Horizon,
           "Invalid HUSR header size 0x{:x} for user {:032x}",
           header.header_size, user_id);

    // Data
    const auto base = reader.Read<ProfileBase>();
    const auto data = reader.Read<UserData>();
    const auto avatar_bg_color = reader.Read<uchar3>();
    std::string avatar_path;
    {
        const auto size = reader.Read<u32>();
        avatar_path.resize(size);
        reader.ReadPtr(avatar_path.data(), size);
    }

    User user(base, data, avatar_bg_color, avatar_path);
    users.insert({user_id, {user, user.GetLastEditTimestamp()}});
}

void UserManager::PreloadAvatar(Avatar& avatar, bool is_compressed) {
    if (!avatar.data.empty())
        return;

    auto stream = avatar.file->Open(filesystem::FileOpenFlags::Read);
    auto reader = stream.CreateReader();

    std::vector<u8> raw(reader.GetSize());
    reader.ReadWhole<u8>(raw.data());

    avatar.file->Close(stream);

    if (is_compressed) {
        // Decompress
#define YAZ0_ASSERT(expr)                                                      \
    {                                                                          \
        const auto res = expr;                                                 \
        ASSERT(res == YAZ0_OK, Services, #expr " failed: {}", res);            \
    }
        Yaz0Stream* yaz0;
        YAZ0_ASSERT(yaz0Init(&yaz0));
        YAZ0_ASSERT(yaz0ModeDecompress(yaz0));
        YAZ0_ASSERT(yaz0Input(yaz0, raw.data(), static_cast<u32>(raw.size())));
        avatar.data.resize(AVATAR_UNCOMPRESSED_IMAGE_SIZE);
        YAZ0_ASSERT(yaz0Output(yaz0, avatar.data.data(),
                               AVATAR_UNCOMPRESSED_IMAGE_SIZE));
        YAZ0_ASSERT(yaz0Run(yaz0));
        YAZ0_ASSERT(yaz0Destroy(yaz0));
#undef YAZ0_ASSERT

        avatar.dimensions = AVATAR_IMAGE_DIMENSIONS;
    } else {
        // Load with STB image
        int width, height;
        auto pixels =
            stbi_load_from_memory(raw.data(), static_cast<i32>(raw.size()),
                                  &width, &height, nullptr, 4);
        if (!pixels) {
            LOG_ERROR(Services, "Failed to load avatar image");
            return;
        }

        // TODO: crop the image if the dimensions don't match
        ASSERT(width == height, Services,
               "Avatar image is not a sqaure ({}x{})", width, height);
        avatar.dimensions = width;

        // TODO: avoid intermediate copy
        usize size = avatar.dimensions * avatar.dimensions * 4;
        avatar.data.resize(size);
        std::memcpy(avatar.data.data(), pixels, size);
        stbi_image_free(pixels);
    }
}

} // namespace hydra::horizon::services::account::internal
