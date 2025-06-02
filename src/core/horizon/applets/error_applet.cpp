#include "core/horizon/applets/error_applet.hpp"

#include "core/horizon/os.hpp"
#include "core/horizon/ui/handler_base.hpp"

namespace hydra::horizon::applets {

namespace {

enum class ErrorType : u8 {
    SystemData = 0,
    SystemError = 1,
    ApplicationError = 2,
    Eula = 3,
    ParentalControl = 4,
    RecordedSystemData = 5,
    RecordedSystemError = 6,
    RecordedApplicationError = 7,
    SystemUpdateEula = 8,
};

struct ParamCommon {
    ErrorType type;
    bool is_jump_enabled;
    u8 _reserved[2];
};

struct ParamForApplicationError {
    u8 version;
    u8 _reserved[3];
    result_t error_code_number;
    u64 language_code;
    char dialog_message[0x800];
    char fullscreen_message[0x800];
};

enum class JumpDestination : u8 {
    Nowhere = 0,
    Set_SystemUpdate = 1,
    Set_DataManagement = 2,
    Set_InternetSetting = 3,
    Cabinet_NfpDataSetting = 4,
};

struct ReturnValue {
    u8 version{0};
    JumpDestination destination{JumpDestination::Nowhere};
};

} // namespace

} // namespace hydra::horizon::applets

ENABLE_ENUM_FORMATTING(hydra::horizon::applets::ErrorType, SystemData,
                       "system data", SystemError, "system error",
                       ApplicationError, "application error", Eula, "eula",
                       ParentalControl, "parental control", RecordedSystemData,
                       "recorded system data", RecordedSystemError,
                       "recorded system error", RecordedApplicationError,
                       "recorded application error", SystemUpdateEula,
                       "system update eula")

namespace hydra::horizon::applets {

result_t ErrorApplet::Run() {
    auto param_reader = PopInDataRaw();
    const auto param_common = param_reader.Read<ParamCommon>();

    // TODO: context (if present)

    ReturnValue ret;
    switch (param_common.type) {
    case ErrorType::ApplicationError: {
        const auto param = param_reader.Read<ParamForApplicationError>();

        // TODO: handle empty messages
        OS_INSTANCE.GetUiHandler().ShowMessageDialog(
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

} // namespace hydra::horizon::applets
