#include "core/horizon/services/pl/sharedresource/platform_shared_resource_manager.hpp"

namespace hydra::horizon::services::pl::sharedresource {

DEFINE_SERVICE_COMMAND_TABLE(IPlatformSharedResourceManager, 0, RequestLoad, 1,
                             GetLoadState, 2, GetSize, 3,
                             GetSharedMemoryAddressOffset, 4,
                             GetSharedMemoryNativeHandle, 5,
                             GetSharedFontInOrderOfPriority, 6,
                             GetSharedFontInOrderOfPriorityForSystem)

} // namespace hydra::horizon::services::pl::sharedresource
