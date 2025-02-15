#include "horizon/services/hosbinder/hos_binder_driver.hpp"

#include "horizon/os.hpp"

namespace Hydra::Horizon::Services::HosBinder {

enum class BinderType : i32 {
    Weak = 0,
    Strong = 1,
};

struct AdjustRefcountIn {
    i32 binder_id;
    i32 addval;
    BinderType type;
};

DEFINE_SERVICE_COMMAND_TABLE(IHOSBinderDriver, 1, AdjustRefcount)

void IHOSBinderDriver::AdjustRefcount(REQUEST_COMMAND_PARAMS) {
    auto in = readers.reader.Read<AdjustRefcountIn>();
    auto& binder =
        OS::GetInstance().GetDisplayBinderManager().GetBinder(in.binder_id);
    switch (in.type) {
    case BinderType::Weak:
        binder.weak_ref_count += in.addval;
        break;
    case BinderType::Strong:
        binder.strong_ref_count += in.addval;
        break;
    }
}

} // namespace Hydra::Horizon::Services::HosBinder
