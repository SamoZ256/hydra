#include "core/horizon/applets/software_keyboard/applet.hpp"

#include "core/horizon/ui/handler_base.hpp"
#include "core/system.hpp"

namespace hydra::horizon::applets::software_keyboard {

namespace {

enum class TextCheckResult : u32 {
    Success = 0,
    ShowFailureDialog = 1,
    ShowConfirmDialog = 2,
};

} // namespace

result_t Applet::Run(System& system) {
    const auto config = PopInData<KeyboardConfigCommon>();
    // TODO: work buffer

    // TODO: additional config

    SoftwareKeyboardResult result;
    std::u16string output_text;
    while (true) {
        // Text input
        std::string output_text_utf8;
        result = system.GetUIHandler().ShowSoftwareKeyboard(
            utf16_to_utf8(std::u16string(config.header_text)),
            utf16_to_utf8(std::u16string(config.sub_text)),
            utf16_to_utf8(std::u16string(config.guide_text)), output_text_utf8);
        output_text = utf8_to_utf16(output_text_utf8);
        if (!config.text_check_enabled)
            break;

        // Verify
        usize size = sizeof(u64) + (output_text.size() + 1) * sizeof(char16_t);
        auto ptr = reinterpret_cast<u8*>(malloc(size));
        io::MemoryStream stream(std::span(ptr, size));
        stream.Write<u64>(size);
        stream.WriteSpan(std::span<const char16_t>(output_text));
        stream.Write(u'\0');
        PushInteractiveOutDataRaw(sized_ptr(ptr, size));

        auto reader = PopInteractiveInDataRaw();
        auto res = reader.Read<TextCheckResult>();
        if (res == TextCheckResult::Success)
            break;

        // Dialog
        std::u16string msg = reader.ReadPtr<char16_t>();
        system.GetUIHandler().ShowMessageDialog(
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
        auto ptr = reinterpret_cast<u8*>(std::malloc(size));
        io::MemoryStream stream(std::span(ptr, size));
        stream.Write(result);
        stream.WriteSpan(std::span<const char16_t>(output_text));
        stream.Write(u'\0');
        PushOutDataRaw(sized_ptr(ptr, size));
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::applets::software_keyboard
