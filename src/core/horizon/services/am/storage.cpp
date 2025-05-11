#include "core/horizon/services/am/storage.hpp"

#include "core/horizon/services/am/storage_accessor.hpp"

namespace Hydra::Horizon::Services::Am {

DEFINE_SERVICE_COMMAND_TABLE(IStorage, 0, Open)

result_t IStorage::Open(add_service_fn_t add_service) {
    add_service(new IStorageAccessor(data));
    return RESULT_SUCCESS;
}

} // namespace Hydra::Horizon::Services::Am
