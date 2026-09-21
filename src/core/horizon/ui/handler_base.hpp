#pragma once

#include "core/horizon/applets/software_keyboard/const.hpp"
#include "core/horizon/ui/const.hpp"

namespace hydra::horizon::ui {

class IHandler {
  public:
    virtual ~IHandler() = default;

    // TODO: detail
    virtual void showMessageDialog(const MessageDialogType type,
                                   const std::string& title,
                                   const std::string& message) = 0;
    virtual applets::software_keyboard::SoftwareKeyboardResult
    showSoftwareKeyboard(const std::string& header_text,
                         const std::string& sub_text,
                         const std::string& guide_text,
                         std::string& out_text) = 0;
};

} // namespace hydra::horizon::ui
