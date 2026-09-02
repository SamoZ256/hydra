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
            Utf16ToUtf8(std::u16string(config.header_text)).value_or(""),
            Utf16ToUtf8(std::u16string(config.sub_text)).value_or(""),
            Utf16ToUtf8(std::u16string(config.guide_text)).value_or(""),
            output_text_utf8);
        const auto output_text_opt = Utf8ToUtf16(output_text_utf8);
        ASSERT(output_text_opt.has_value(), Applets,
               "Failed to convert UTF-8 to UTF-16: {}", output_text_utf8);
        output_text = output_text_opt.value();

        if (!config.text_check_enabled)
            break;

        // Verify
        const usize size =
            sizeof(u64) + ((output_text.size() + 1) * sizeof(char16_t));
        std::vector<u8> bytes(size);
        ztd::io::MemoryStream stream(bytes);
        stream.write<u64>(size);
        stream.writeSpan(std::span<const char16_t>(output_text));
        stream.write(u'\0');
        PushInteractiveOutDataRaw(std::move(bytes));

        auto reader = PopInteractiveInDataRaw();
        auto res = reader.read<TextCheckResult>();
        if (res == TextCheckResult::Success)
            break;

        // Dialog
        std::u16string msg = reader.readPtr<char16_t>();
        system.GetUIHandler().ShowMessageDialog(
            (res == TextCheckResult::ShowFailureDialog
                 ? ui::MessageDialogType::Error
                 : ui::MessageDialogType::Info),
            "Text input", // TODO: better text
            Utf16ToUtf8(msg).value_or(""));
    }

    // Output
    {
        const usize size = sizeof(SoftwareKeyboardResult) +
                           ((output_text.size() + 1) * sizeof(char16_t));
        std::vector<u8> bytes(size);
        ztd::io::MemoryStream stream(bytes);
        stream.write(result);
        stream.writeSpan(std::span<const char16_t>(output_text));
        stream.write(u'\0');
        PushOutDataRaw(std::move(bytes));
    }

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::applets::software_keyboard
