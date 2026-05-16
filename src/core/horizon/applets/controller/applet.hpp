#pragma once

#include "core/horizon/applets/applet_base.hpp"
#include "core/horizon/applets/controller/const.hpp"
#include "core/horizon/os.hpp"
#include "core/horizon/ui/handler_base.hpp"

namespace hydra::horizon::applets::controller {

class Applet : public AppletBase {
  public:
    using AppletBase::AppletBase;

  protected:
    result_t Run(System& system) override;

  private:
    template <usize max_supported_players>
    result_t RunControllerSupport(System& system) {
        const auto arg = PopInData<SupportArg<max_supported_players>>();

        (void)system;
        LOG_FUNC_WITH_ARGS_STUBBED(
            Applets,
            "player count min: {}, player count max: {}, single mode: {}, "
            "permit joy dual: {}, explain text: {}",
            arg.player_count_min, arg.player_count_max, arg.enable_single_mode,
            arg.enable_permit_joy_dual,
            arg.enable_explain_text ? arg.explain_text[0] : "none");

        i8 player_count_min = arg.player_count_min;
        i8 player_count_max = arg.player_count_max;
        if (arg.enable_single_mode)
            player_count_min = player_count_max = 1;

        // HACK
        PushOutData(ResultInfoInternal{
            .info =
                {
                    .player_count = player_count_min,
                    .selected_id = services::hid::NpadIdType::No1,
                },
            .result = RESULT_SUCCESS,
        });

        return RESULT_SUCCESS;
    }
};

} // namespace hydra::horizon::applets::controller
