#include "core/horizon/applets/player_select/applet.hpp"

#include "core/horizon/applets/player_select/const.hpp"
#include "core/system.hpp"

namespace hydra::horizon::applets::player_select {

result_t Applet::run(System& system) {
    auto settings_reader = popInDataRaw();
    const auto settings = settings_reader.read<UiSettings>();
    (void)settings;

    // This should give the user an option to select a profile to use. However,
    // we require the user to select it before starting the game, so we can just
    // return that
    UiReturnArg ret{
        .result = RESULT_SUCCESS,
        .selected_user = system.getOs()
                             .getUserManager()
                             .getUserIDs()[0], // TODO: use the active user ID
    };

    pushOutData(ret);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::applets::player_select
