#include "horizon/services/visrv/application_display_service.hpp"

#include "horizon/cmif.hpp"
#include "horizon/horizon.hpp"
#include "horizon/kernel.hpp"
#include "horizon/services/hosbinder/hos_binder_driver.hpp"
#include "horizon/services/visrv/manager_display_service.hpp"
#include "horizon/services/visrv/system_display_service.hpp"
#include "hw/bus.hpp"
#include "hw/display/display.hpp"

namespace Hydra::Horizon::Services::ViSrv {

struct ParcelData {
    u32 unknown0;
    u32 unknown1;
    u32 binder_id;
    u32 unknown2[3];
    u64 str;
    u64 unknown3;
};

struct Parcel {
    u32 data_size;
    u32 data_offset;
    u32 objects_size;
    u32 objects_offset;
};

struct OpenLayerIn {
    u64 display_name;
    u64 layer_id;
    u64 applet_resource_user_id;
};

void IApplicationDisplayService::Request(
    Readers& readers, Writers& writers,
    std::function<void(ServiceBase*)> add_service) {
    auto cmif_in = readers.reader.Read<Cmif::InHeader>();

    Result* res = Cmif::write_out_header(writers.writer);

    switch (cmif_in.command_id) {
    case 100: // GetRelayService
        hos_binder_driver = new HosBinder::IHOSBinderDriver();
        add_service(hos_binder_driver);
        break;
    case 101: // GetSystemDisplayService
        add_service(new ISystemDisplayService());
        break;
    case 102: // GetManagerDisplayService
        add_service(new IManagerDisplayService());
        break;
    case 1010: {            // OpenDisplay
        u64 display_id = 0; // TODO: get based on the name
        Kernel::GetInstance().GetBus().GetDisplay(display_id)->Open();
        writers.writer.Write(display_id);
        break;
    }
    case 1020: { // CloseDisplay
        u64 display_id = readers.reader.Read<u64>();
        Kernel::GetInstance().GetBus().GetDisplay(display_id)->Close();
        break;
    }
    case 2020: { // OpenLayer
        auto in = readers.reader.Read<OpenLayerIn>();

        // Out
        // TODO: output window size

        // Parcel
        Parcel parcel{
            .data_size = sizeof(ParcelData),
            .data_offset = sizeof(Parcel),
            .objects_size = 0,
            .objects_offset = 0,
        };
        writers.revc_buffers_writer.Write(parcel);

        // Parcel data
        ParcelData data{
            .binder_id = hos_binder_driver->AddBinder(),
        };
        writers.revc_buffers_writer.Write(data);

        break;
    }
    default:
        LOG_WARNING(HorizonServices, "Unknown request {}", cmif_in.command_id);
        break;
    }

    *res = RESULT_SUCCESS;
}

} // namespace Hydra::Horizon::Services::ViSrv
