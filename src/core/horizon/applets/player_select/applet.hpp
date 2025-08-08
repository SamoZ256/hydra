#pragma once

#include "core/horizon/applets/applet_base.hpp"

namespace hydra::horizon::applets::player_select {

class Applet : public AppletBase {
  public:
    using AppletBase::AppletBase;

  protected:
    result_t Run() override;
};

} // namespace hydra::horizon::applets::player_select
