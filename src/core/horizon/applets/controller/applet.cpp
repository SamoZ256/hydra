#include "core/horizon/applets/controller/applet.hpp"

namespace hydra::horizon::applets::controller {

result_t Applet::run(System& system) {
    const auto arg_private = popInData<ArgPrivate>();

    // Mode
    if (arg_private.mode != Mode::ShowControllerSupport) {
        LOG_WARN(Applets, "Unimplemented mode {}", arg_private.mode);

        // Dummy response
        pushOutData(ResultInfoInternal{
            .info = {},
            .result = RESULT_SUCCESS,
        });
        return RESULT_SUCCESS;
    }

    // Run with the correct args
    switch (arg_private.controller_support_arg_size) {
    case sizeof(SupportArg<4>):
        return runControllerSupport<4>(system);
    case sizeof(SupportArg<8>):
        return runControllerSupport<8>(system);
    default:
        LOG_ERROR(Applets, "Invalid controller support argument size 0x{:x}",
                  arg_private.controller_support_arg_size);
        return MAKE_RESULT(Svc, 2); // TODO
    }
}

} // namespace hydra::horizon::applets::controller
