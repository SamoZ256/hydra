#include "core/horizon/services/session.hpp"

#include "core/horizon/cmif.hpp"
#include "core/horizon/const.hpp"
#include "core/horizon/kernel.hpp"
#include "core/horizon/services/domain_service.hpp"
#include "core/hw/tegra_x1/cpu/mmu_base.hpp"

namespace Hydra::Horizon::Services {

void Session::Close() {
    // TODO: correct?
    // delete service;
    service = nullptr;
}

void Session::Request(REQUEST_PARAMS) {
    service->Request(readers, writers, add_service);
}

void Session::Control(Readers& readers, Writers& writers) {
    auto cmif_in = readers.reader.Read<Cmif::InHeader>();

    Result* result = Cmif::write_out_header(writers.writer);
    *result = RESULT_SUCCESS;

    const auto command =
        static_cast<Cmif::ControlCommandType>(cmif_in.command_id);
    LOG_DEBUG(HorizonServices, "Control request {}", command);
    switch (command) {
    case Cmif::ControlCommandType::ConvertCurrentObjectToDomain: {
        auto domain_service = new DomainService();
        handle_id = domain_service->AddObject(service);
        service = domain_service;

        // Out
        writers.writer.Write(handle_id);

        break;
    }
    case Cmif::ControlCommandType::CloneCurrentObject: { // clone current object
        auto clone = new Session(service);
        handle_id_t handle_id = Kernel::GetInstance().AddHandle(clone);
        clone->SetHandleId(handle_id);
        writers.move_handles_writer.Write(handle_id);
        break;
    }
    case Cmif::ControlCommandType::QueryPointerBufferSize: // query pointer
                                                           // buffer size
        writers.writer.Write(service->GetPointerBufferSize());
        break;
    case Cmif::ControlCommandType::CloneCurrentObjectEx: { // clone current ex
        // TODO: u32 tag
        auto clone = new Session(service);
        handle_id_t handle_id = Kernel::GetInstance().AddHandle(clone);
        clone->SetHandleId(handle_id);
        writers.move_handles_writer.Write(handle_id);
        break;
    }
    default:
        LOG_ERROR(HorizonServices, "Unimplemented control request {}", command);
        break;
    }
}

} // namespace Hydra::Horizon::Services
