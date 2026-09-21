#include "core/horizon/services/am/storage.hpp"

#include "core/horizon/services/am/storage_accessor.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IStorage, 0, open)

result_t IStorage::open(RequestContext* ctx) {
    addService(*ctx, new IStorageAccessor(data));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
