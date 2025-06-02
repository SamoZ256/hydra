#include "core/horizon/kernel/session.hpp"

#include "core/horizon/kernel/cmif.hpp"
#include "core/horizon/kernel/domain_service.hpp"
#include "core/horizon/kernel/kernel.hpp"

namespace hydra::horizon::kernel {

void Session::Close() {
    // TODO: ref counting?
    // delete service;
    service = nullptr;
}

void Session::Request(RequestContext& context) { service->Request(context); }

void Session::Control(hipc::Readers& readers, hipc::Writers& writers) {
    auto cmif_in = readers.reader.Read<cmif::InHeader>();

    result_t* result = cmif::write_out_header(writers.writer);

    const auto command =
        static_cast<cmif::ControlCommandType>(cmif_in.command_id);
    LOG_DEBUG(Services, "Control request {}", command);
    switch (command) {
    case cmif::ControlCommandType::ConvertCurrentObjectToDomain: {
        auto domain_service = new DomainService();
        handle_id = domain_service->AddSubservice(service);
        service = domain_service;

        // Out
        writers.writer.Write(handle_id);

        break;
    }
    case cmif::ControlCommandType::CloneCurrentObject: { // clone current object
        auto clone = new Session(service);
        handle_id_t handle_id = KERNEL_INSTANCE.AddHandle(clone);
        clone->SetHandleId(handle_id);
        writers.move_handles_writer.Write(handle_id);
        break;
    }
    case cmif::ControlCommandType::QueryPointerBufferSize: // query pointer
                                                           // buffer size
        writers.writer.Write(service->GetPointerBufferSize());
        break;
    case cmif::ControlCommandType::CloneCurrentObjectEx: { // clone current ex
        // TODO: u32 tag
        auto clone = new Session(service);
        handle_id_t handle_id = KERNEL_INSTANCE.AddHandle(clone);
        clone->SetHandleId(handle_id);
        writers.move_handles_writer.Write(handle_id);
        break;
    }
    default:
        LOG_ERROR(Services, "Unimplemented control request {}", command);
        break;
    }
}

} // namespace hydra::horizon::kernel
