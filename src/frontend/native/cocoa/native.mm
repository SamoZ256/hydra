#include "frontend/native/cocoa/native.hpp"

#import <Cocoa/Cocoa.h>
#import <dispatch/dispatch.h>

using TextInputCallback = void (*)(const char* text, void* user_data);

@interface TextInputDelegate : NSObject <NSTextFieldDelegate>
@property(strong) NSAlert* alert;
@property(strong) NSTextField* textField;
@property(assign) TextInputCallback callback;
@property(assign) void* userData;
@end

@implementation TextInputDelegate

- (void)showTextInputDialog:(NSString*)title
                placeholder:(NSString*)placeholder
                   callback:(TextInputCallback)callback
                   userData:(void*)userData {

    self.callback = callback;
    self.userData = userData;

    // Create the alert dialog
    self.alert = [[NSAlert alloc] init];
    [self.alert setMessageText:title];
    [self.alert setInformativeText:@"Enter text:"];
    [self.alert addButtonWithTitle:@"OK"];
    [self.alert addButtonWithTitle:@"Cancel"];

    // Create text field
    self.textField =
        [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
    [self.textField setPlaceholderString:placeholder];
    [self.textField setDelegate:self];

    // Set the text field as accessory view
    [self.alert setAccessoryView:self.textField];

    // Make text field first responder
    [[self.alert window] makeFirstResponder:self.textField];

    // Show the dialog
    NSModalResponse response = [self.alert runModal];

    // Handle response
    if (response == NSAlertFirstButtonReturn) {
        // OK button pressed
        NSString* text = [self.textField stringValue];
        const char* cText = [text UTF8String];

        if (self.callback != nullptr) {
            self.callback(cText, self.userData);
        }
    } else {
        // Cancel button pressed or dialog closed
        if (self.callback != nullptr) {
            self.callback(nullptr, self.userData);
        }
    }
}

// Handle Enter key in text field
- (void)controlTextDidEndEditing:(NSNotification*)notification {
    NSTextField* textField = [notification object];
    if (textField == self.textField) {
        // User pressed Enter - same as clicking OK
        [NSApp stopModalWithCode:NSAlertFirstButtonReturn];
    }
}

@end

namespace hydra::frontend::native::cocoa {

namespace {

struct Result {
    std::mutex mutex;
    std::condition_variable cond_var;
    std::string text;
    bool ok;
};

void textInputCallback(const char* text, void* user_data) {
    auto result = reinterpret_cast<Result*>(user_data);

    std::unique_lock lock(result->mutex);

    if (text != nullptr) {
        result->text = text;
        result->ok = true;
    } else {
        result->ok = false;
    }

    // TODO: notify just one?
    result->cond_var.notify_all();
}

} // namespace

Native::Native() { text_input_delegate = [[TextInputDelegate alloc] init]; }

Native::~Native() { [text_input_delegate release]; }

bool Native::showInputTextDialog(const std::string& header_text,
                                 const std::string& sub_text,
                                 const std::string& guide_text,
                                 std::string& out_text) {
    NSString* ns_title =
        [NSString stringWithUTF8String:(header_text + "\n" + sub_text).c_str()];
    NSString* ns_placeholder =
        [NSString stringWithUTF8String:guide_text.c_str()];

    Result result;
    auto result_ptr =
        &result; // Take a reference to result to not confuse the compiler
    dispatch_async(dispatch_get_main_queue(), ^{
      [text_input_delegate showTextInputDialog:ns_title
                                   placeholder:ns_placeholder
                                      callback:textInputCallback
                                      userData:result_ptr];
    });

    std::unique_lock lock(result.mutex);
    result.cond_var.wait(lock);

    out_text = result.text;
    return result.ok;
}

} // namespace hydra::frontend::native::cocoa
