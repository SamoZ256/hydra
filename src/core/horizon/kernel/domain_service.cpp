#include "core/horizon/kernel/domain_service.hpp"

#include "core/horizon/kernel/cmif.hpp"
#include "core/horizon/kernel/service_base.hpp"

namespace hydra::horizon::kernel {

void DomainService::Request(RequestContext& context) {
    // LOG_DEBUG(Services, "Domain service request");

    // Domain in
    auto cmif_in = context.readers.reader.Read<cmif::DomainInHeader>();
    // LOG_DEBUG(Services, "Object ID: 0x{:08x}", cmif_in.object_id);
    auto subservice = subservice_pool.Get(cmif_in.object_id);

    if (cmif_in.num_in_objects != 0) {
        auto objects = context.readers.reader.GetPtr() + cmif_in.data_size;
        context.readers.objects_reader =
            new Reader(objects, cmif_in.num_in_objects * sizeof(handle_id_t));
    }

    cmif::write_domain_out_header(context.writers.writer);

    switch (cmif_in.type) {
    case cmif::DomainCommandType::SendMessage: {
        RequestContext subcontext{
            context.readers,
            context.writers,
            [&](ServiceBase* service) {
                handle_id_t handle_id = subservice_pool.Add(service);
                context.writers.objects_writer.Write(handle_id);
            },
            [&](handle_id_t handle_id) {
                return subservice_pool.Get(handle_id);
            },
        };
        subservice->Request(subcontext);
        break;
    }
    case cmif::DomainCommandType::Close:
        subservice_pool.Free(cmif_in.object_id);
        LOG_DEBUG(Kernel, "Closed subservice");
        break;
    default:
        LOG_WARN(Kernel, "Unknown domain request type {}", cmif_in.type);
        break;
    }
}

} // namespace hydra::horizon::kernel
