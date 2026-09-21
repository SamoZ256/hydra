#include "core/horizon/services/pl/sharedresource/platform_shared_resource_manager.hpp"

namespace hydra::horizon::services::pl::sharedresource {

DEFINE_SERVICE_COMMAND_TABLE(IPlatformSharedResourceManager, 0, requestLoad, 1,
                             getLoadState, 2, getSize, 3,
                             getSharedMemoryAddressOffset, 4,
                             getSharedMemoryNativeHandle, 5,
                             getSharedFontInOrderOfPriority, 6,
                             getSharedFontInOrderOfPriorityForSystem)

} // namespace hydra::horizon::services::pl::sharedresource
