#include "core/input/apple_gc/device_list.hpp"

#import <GameController/GameController.h>

#include "core/input/apple_gc/controller.hpp"
#include "core/input/apple_gc/keyboard.hpp"

using DeviceList = hydra::input::apple_gc::DeviceList;

@interface DeviceListImpl : NSObject

@property DeviceList* parent;

@end

@implementation DeviceListImpl

- (id)initWithParent:(DeviceList*)parent {
    if (self = [super init]) {
        self.parent = parent;

        // Notifications
        [[NSNotificationCenter defaultCenter]
            addObserver:self
               selector:@selector(controllerConnected:)
                   name:GCControllerDidConnectNotification
                 object:nil];
        [[NSNotificationCenter defaultCenter]
            addObserver:self
               selector:@selector(controllerDisconnected:)
                   name:GCControllerDidDisconnectNotification
                 object:nil];
        [[NSNotificationCenter defaultCenter]
            addObserver:self
               selector:@selector(keyboardConnected:)
                   name:GCKeyboardDidConnectNotification
                 object:nil];
        [[NSNotificationCenter defaultCenter]
            addObserver:self
               selector:@selector(keyboardDisconnected:)
                   name:GCKeyboardDidDisconnectNotification
                 object:nil];

        // Connected keyboards
        if (@available(macOS 11.0, iOS 14.0, tvOS 14.0, *)) {
            GCKeyboard* keyboard = [GCKeyboard coalescedKeyboard];
            if (keyboard) {
                self.parent->AddKeyboard(keyboard);
            }
        }

        // Connected controllers
        for (GCController* controller in [GCController controllers]) {
            self.parent->AddController(controller);
        }
    }

    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void)controllerConnected:(NSNotification*)notification {
    GCController* controller =
        reinterpret_cast<GCController*>(notification.object);
    _parent->AddController(controller);
}

- (void)controllerDisconnected:(NSNotification*)notification {
    GCController* controller =
        reinterpret_cast<GCController*>(notification.object);
    _parent->RemoveController(controller);
}

- (void)keyboardConnected:(NSNotification*)notification {
    GCKeyboard* keyboard = reinterpret_cast<GCKeyboard*>(notification.object);
    _parent->AddKeyboard(keyboard);
}

- (void)keyboardDisconnected:(NSNotification*)notification {
    GCKeyboard* keyboard = reinterpret_cast<GCKeyboard*>(notification.object);
    _parent->RemoveKeyboard(keyboard);
}

@end

namespace hydra::input::apple_gc {

namespace {

std::string GetDeviceName(id device) {
    return [[device vendorName] UTF8String];
}

} // namespace

DeviceList::DeviceList() {
    impl = [[DeviceListImpl alloc] initWithParent:this];
}

DeviceList::~DeviceList() { [impl release]; }

void DeviceList::AddController(id controller) {
    AddDevice(GetDeviceName(controller), new Controller(controller));
}

void DeviceList::RemoveController(id controller) {
    RemoveDevice(GetDeviceName(controller));
}

void DeviceList::AddKeyboard(id keyboard) {
    AddDevice(GetDeviceName(keyboard), new Keyboard(keyboard));
}

void DeviceList::RemoveKeyboard(id keyboard) {
    RemoveDevice(GetDeviceName(keyboard));
}

} // namespace hydra::input::apple_gc
