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
    _parent->_AddController(controller);
}

- (void)controllerDisconnected:(NSNotification*)notification {
    GCController* controller =
        reinterpret_cast<GCController*>(notification.object);
    _parent->_RemoveController(controller);
}

- (void)keyboardConnected:(NSNotification*)notification {
    GCKeyboard* keyboard = reinterpret_cast<GCKeyboard*>(notification.object);
    _parent->_AddKeyboard(keyboard);
}

- (void)keyboardDisconnected:(NSNotification*)notification {
    GCKeyboard* keyboard = reinterpret_cast<GCKeyboard*>(notification.object);
    _parent->_RemoveKeyboard(keyboard);
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

void DeviceList::_AddController(id controller) {
    const auto name = GetDeviceName(controller);
    if (!HasDevice(name))
        AddDevice(name, new Controller(controller));
}

void DeviceList::_RemoveController(id controller) {
    RemoveDevice(GetDeviceName(controller));
}

void DeviceList::_AddKeyboard(id keyboard) {
    const auto name = GetDeviceName(keyboard);
    if (!HasDevice(name))
        AddDevice(name, new Keyboard(keyboard));
}

void DeviceList::_RemoveKeyboard(id keyboard) {
    RemoveDevice(GetDeviceName(keyboard));
}

} // namespace hydra::input::apple_gc
