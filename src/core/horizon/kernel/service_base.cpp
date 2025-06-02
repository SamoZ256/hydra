#include "core/horizon/kernel/service_base.hpp"

#include "core/horizon/kernel/cmif.hpp"

namespace hydra::horizon::kernel {

void ServiceBase::Request(RequestContext& context) {
    auto cmif_in = context.readers.reader.Read<cmif::InHeader>();

    result_t* result = cmif::write_out_header(context.writers.writer);

    *result = RequestImpl(context, cmif_in.command_id);
}

} // namespace hydra::horizon::kernel
