#include "core/horizon/applets/err/error_applet.hpp"

#include "core/horizon/applets/err/const.hpp"
#include "core/horizon/os.hpp"
#include "core/horizon/ui/handler_base.hpp"

namespace hydra::horizon::applets::err {

result_t ErrorApplet::Run() {
    auto param_reader = PopInDataRaw();
    const auto param_common = param_reader.Read<ParamCommon>();

    // TODO: context (if present)

    ReturnValue ret;
    switch (param_common.type) {
    case ErrorType::ApplicationError: {
        const auto param = param_reader.Read<ParamForApplicationError>();

        // TODO: handle empty messages
        OS_INSTANCE.GetUIHandler().ShowMessageDialog(
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

} // namespace hydra::horizon::applets::err
