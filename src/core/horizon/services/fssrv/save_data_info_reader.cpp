#include "core/horizon/services/fssrv/save_data_info_reader.hpp"

namespace hydra::horizon::services::fssrv {

namespace {

struct SaveDataInfo {
    u64 id;
    SaveDataSpaceId space_id;
    SaveDataType type;
    u8 _padding[6];
    uuid_t uid;
    u64 system_save_data_id;
    u64 application_id;
    u64 save_image_size;
    u16 save_data_index;
    SaveDataRank rank;
    u8 _unknown_0x3b[0x25];
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(ISaveDataInfoReader, 0, ReadSaveDataInfo)

result_t ISaveDataInfoReader::ReadSaveDataInfo(
    i64* out_entry_count, OutBuffer<BufferAttr::MapAlias> out_entry_buffer) {
    // TODO: don't hardcode the entries
    static SaveDataInfo entries[0] = {};

    auto stream = out_entry_buffer.stream;
    for (u32 i = entry_index; i < sizeof_array(entries); i++) {
        if (stream->getSeek() + sizeof(SaveDataInfo) > stream->getSize())
            break;

        stream->write(entries[i]);
    }

    *out_entry_count = stream->getSeek() / sizeof(SaveDataInfo);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
