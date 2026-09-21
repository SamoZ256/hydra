#include "core/horizon/services/ns/application_manager_interface.hpp"

namespace hydra::horizon::services::ns {

namespace {

enum class ApplicationEvent : u8 {
    Launched = 0,
    LocalInstalled = 1,
    DownloadStarted = 2,
    GameCardInserted = 3,
    Touched = 4,
    // TODO: more?
};

struct ApplicationRecord {
    u64 id;
    ApplicationEvent last_event;
    u8 attributes; // TODO
    u8 _reserved_xa[6];
    u64 last_updated; // TODO: in what units?
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IApplicationManagerInterface, 0,
                             listApplicationRecord)

result_t IApplicationManagerInterface::listApplicationRecord(
    i32 entry_offset, i32* out_entry_count,
    OutBuffer<BufferAttr::MapAlias> out_record_buffer) {
    (void)out_record_buffer;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "entry offset: {:#x}", entry_offset);

    // HACK
    *out_entry_count = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::ns
