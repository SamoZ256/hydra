#include "core/horizon/services/pl/detail/platform_service_manager_for_system.hpp"

namespace hydra::horizon::services::pl::detail {

DEFINE_SERVICE_COMMAND_TABLE(IPlatformServiceManagerForSystem, 0, requestLoad,
                             1, getLoadState, 2, getSize, 3,
                             getSharedMemoryAddressOffset, 4,
                             getSharedMemoryNativeHandle, 5,
                             getSharedFontInOrderOfPriority)

} // namespace hydra::horizon::services::pl::detail
