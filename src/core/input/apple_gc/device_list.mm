#include "core/input/apple_gc/device_list.hpp"

#import <GameController/GameController.h>

#include "core/input/apple_gc/controller.hpp"
#include "core/input/apple_gc/keyboard.hpp"

using DeviceList = hydra::input::apple_gc::DeviceList;

namespace hydra::input::apple_gc {

namespace {

std::string GetDeviceName(id device) {
    return [[device vendorName] UTF8String];
}

} // namespace

} // namespace hydra::input::apple_gc

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
                self.parent->_AddKeyboard(keyboard);
            }
        }

        // Connected controllers
        for (GCController* controller in [GCController controllers]) {
            self.parent->_AddController(controller);
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
    _parent->AddDevice(hydra::input::apple_gc::GetDeviceName(controller),
                       new hydra::input::apple_gc::Controller(controller));
}

- (void)controllerDisconnected:(NSNotification*)notification {
    GCController* controller =
        reinterpret_cast<GCController*>(notification.object);
    _parent->RemoveDevice(hydra::input::apple_gc::GetDeviceName(controller));
}

- (void)keyboardConnected:(NSNotification*)notification {
    GCKeyboard* keyboard = reinterpret_cast<GCKeyboard*>(notification.object);
    _parent->AddDevice(hydra::input::apple_gc::GetDeviceName(keyboard),
                       new hydra::input::apple_gc::Keyboard(keyboard));
}

- (void)keyboardDisconnected:(NSNotification*)notification {
    GCKeyboard* keyboard = reinterpret_cast<GCKeyboard*>(notification.object);
    _parent->RemoveDevice(hydra::input::apple_gc::GetDeviceName(keyboard));
}

@end

namespace hydra::input::apple_gc {

DeviceList::DeviceList() {
    impl = [[DeviceListImpl alloc] initWithParent:this];
}

DeviceList::~DeviceList() { [impl release]; }

} // namespace hydra::input::apple_gc
