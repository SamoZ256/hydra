#pragma once

#include "core/horizon/applets/applet_base.hpp"

namespace hydra::horizon::applets::error {

class Applet : public AppletBase {
  public:
    using AppletBase::AppletBase;

  protected:
    result_t Run(System& system) override;
};

} // namespace hydra::horizon::applets::error
