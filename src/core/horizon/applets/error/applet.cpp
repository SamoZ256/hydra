#include "core/horizon/applets/error/applet.hpp"

#include "core/horizon/applets/error/const.hpp"
#include "core/horizon/ui/handler_base.hpp"
#include "core/system.hpp"

namespace hydra::horizon::applets::error {

result_t Applet::Run(System& system) {
    // TODO: PopInData
    auto param_reader = PopInDataRaw();
    const auto param_common = param_reader.Read<ParamCommon>();

    // TODO: context (if present)

    ReturnValue ret;
    switch (param_common.type) {
    case ErrorType::ApplicationError: {
        const auto param = param_reader.Read<ParamForApplicationError>();

        // TODO: handle empty messages
        system.GetUIHandler().ShowMessageDialog(
            ui::MessageDialogType::Error,
            fmt::format("Error (0x{:x})", param.error_code_number),
            fmt::format("{}\n{}", param.dialog_message,
                        param.fullscreen_message));
        // TODO: details

        if (param_common.is_jump_enabled) {
            // TODO: set
        }

        break;
    }
    default:
        LOG_NOT_IMPLEMENTED(Applets, "Error type {}", param_common.type);
        break;
    }

    PushOutData(ret);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::applets::error
