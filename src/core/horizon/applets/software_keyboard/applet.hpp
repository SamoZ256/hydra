#pragma once

#include "core/horizon/applets/applet_base.hpp"
#include "core/horizon/applets/software_keyboard/const.hpp"

namespace hydra::horizon::applets::software_keyboard {

class Applet : public AppletBase {
  public:
    using AppletBase::AppletBase;

  protected:
    result_t Run(System& system) override;
};

} // namespace hydra::horizon::applets::software_keyboard
