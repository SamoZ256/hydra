#include "core/horizon/applets/controller/applet.hpp"

namespace hydra::horizon::applets::controller {

result_t Applet::Run() {
    const auto arg_private = PopInData<ArgPrivate>();

    // Mode
    if (arg_private.mode != Mode::ShowControllerSupport) {
        LOG_WARN(Applets, "Unimplemented mode {}", arg_private.mode);

        // Dummy response
        PushOutData(ResultInfoInternal{
            .info = {},
            .result = RESULT_SUCCESS,
        });
        return RESULT_SUCCESS;
    }

    // Run with the correct args
    switch (arg_private.controller_support_arg_size) {
    case sizeof(SupportArg<4>):
        return RunControllerSupport<4>();
    case sizeof(SupportArg<8>):
        return RunControllerSupport<8>();
    default:
        LOG_ERROR(Applets, "Invalid controller support argument size 0x{:x}",
                  arg_private.controller_support_arg_size);
        return MAKE_RESULT(Svc, 2); // TODO
    }
}

} // namespace hydra::horizon::applets::controller
