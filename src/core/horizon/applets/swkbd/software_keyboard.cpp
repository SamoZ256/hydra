#include "core/horizon/applets/swkbd/software_keyboard.hpp"

#include "core/horizon/os.hpp"
#include "core/horizon/ui/handler_base.hpp"

namespace hydra::horizon::applets::swkbd {

namespace {

enum class TextCheckResult : u32 {
    Success = 0,
    ShowFailureDialog = 1,
    ShowConfirmDialog = 2,
};

} // namespace

result_t SoftwareKeyboard::Run() {
    const auto config = PopInData<KeyboardConfigCommon>();
    // TODO: work buffer

    // TODO: additional config

    SoftwareKeyboardResult result;
    std::u16string output_text;
    while (true) {
        // Text input
        std::string output_text_utf8;
        result = OS_INSTANCE.GetUIHandler().ShowSoftwareKeyboard(
            utf16_to_utf8(std::u16string(config.header_text)),
            output_text_utf8);
        output_text = utf8_to_utf16(output_text_utf8);
        if (!config.text_check_enabled)
            break;

        // Verify
        usize size = sizeof(u64) + (output_text.size() + 1) * sizeof(char16_t);
        auto ptr = (u8*)malloc(size);
        Writer writer(ptr, size);
        writer.Write<u64>(size);
        writer.WritePtr(output_text.data(),
                        output_text.size() * sizeof(char16_t));
        writer.Write(u'\0');
        PushInteractiveOutDataRaw(sized_ptr(ptr, size));

        auto reader = PopInteractiveInDataRaw();
        auto res = reader.Read<TextCheckResult>();
        if (res == TextCheckResult::Success)
            break;

        // Dialog
        std::u16string msg = reader.ReadPtr<char16_t>();
        OS_INSTANCE.GetUIHandler().ShowMessageDialog(
            (res == TextCheckResult::ShowFailureDialog
                 ? ui::MessageDialogType::Error
                 : ui::MessageDialogType::Info),
            "Text input", // TODO: better text
            utf16_to_utf8(msg));
    }

    // Output
    {
        usize size = sizeof(SoftwareKeyboardResult) +
                     (output_text.size() + 1) * sizeof(char16_t);
        auto ptr = (u8*)malloc(size);
        Writer writer(ptr, size);
        writer.Write(result);
        writer.WritePtr(output_text.data(),
                        output_text.size() * sizeof(char16_t));
        writer.Write(u'\0');
        PushOutDataRaw(sized_ptr(ptr, size));
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::applets::swkbd
