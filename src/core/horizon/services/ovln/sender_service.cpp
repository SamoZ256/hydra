#include "core/horizon/services/ovln/sender_service.hpp"

#include "core/horizon/services/ovln/sender.hpp"

namespace hydra::horizon::services::ovln {

DEFINE_SERVICE_COMMAND_TABLE(ISenderService, 0, openSender)

result_t ISenderService::openSender(RequestContext* ctx) {
    addService(*ctx, new ISender());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::ovln
