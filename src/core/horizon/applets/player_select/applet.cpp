#include "core/horizon/applets/player_select/applet.hpp"

#include "core/horizon/applets/player_select/const.hpp"
#include "core/horizon/services/account/user_manager.hpp"

namespace hydra::horizon::applets::player_select {

result_t Applet::Run() {
    auto settings_reader = PopInDataRaw();
    const auto settings = settings_reader.Read<UiSettings>();

    // This should give the user an option to select a profile to use. However,
    // we require the user to select it before starting the game, so we can just
    // return that
    UiReturnArg ret{
        .result = RESULT_SUCCESS,
        .selected_user = USER_MANAGER_INSTANCE
                             .GetUserIDs()[0], // TODO: use the active user ID
    };

    PushOutData(ret);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::applets::player_select
