#pragma once

#include "core/horizon/ui/const.hpp"

namespace hydra::horizon::ui {

class HandlerBase {
  public:
    virtual void ShowMessageDialog(const MessageDialogType type,
                                   const std::string& title,
                                   const std::string& message) = 0;
};

} // namespace hydra::horizon::ui
