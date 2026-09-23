#include "core/input/apple_gc/device_list.hpp"

#import <GameController/GameController.h>

#include "core/input/apple_gc/controller.hpp"
#include "core/input/apple_gc/keyboard.hpp"

using hydra::input::apple_gc::DeviceList;

@interface DeviceListImpl : NSObject

@property DeviceList* parent;

@end

@implementation DeviceListImpl

- (id)initWithParent:(DeviceList*)parent {
    self = [super init];
    if (self != nullptr) {
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
            if (keyboard != nullptr) {
                self.parent->addKeyboard(keyboard);
            }
        }

        // Connected controllers
        for (GCController* controller in [GCController controllers]) {
            self.parent->addController(controller);
        }
    }

    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [super dealloc];
}

- (void)controllerConnected:(NSNotification*)notification {
    auto controller = reinterpret_cast<GCController*>(notification.object);
    _parent->addController(controller);
}

- (void)controllerDisconnected:(NSNotification*)notification {
    auto controller = reinterpret_cast<GCController*>(notification.object);
    _parent->removeController(controller);
}

- (void)keyboardConnected:(NSNotification*)notification {
    auto keyboard = reinterpret_cast<GCKeyboard*>(notification.object);
    _parent->addKeyboard(keyboard);
}

- (void)keyboardDisconnected:(NSNotification*)notification {
    auto keyboard = reinterpret_cast<GCKeyboard*>(notification.object);
    _parent->removeKeyboard(keyboard);
}

@end

namespace hydra::input::apple_gc {

namespace {

std::string getDeviceName(id device) {
    return [[device vendorName] UTF8String];
}

} // namespace

DeviceList::DeviceList() : impl([[DeviceListImpl alloc] initWithParent:this]) {}

DeviceList::~DeviceList() { [impl release]; }

void DeviceList::addController(id controller) {
    addDevice(getDeviceName(controller), new Controller(controller));
}

void DeviceList::removeController(id controller) {
    removeDevice(getDeviceName(controller));
}

void DeviceList::addKeyboard(id keyboard) {
    addDevice(getDeviceName(keyboard), new Keyboard(keyboard));
}

void DeviceList::removeKeyboard(id keyboard) {
    removeDevice(getDeviceName(keyboard));
}

} // namespace hydra::input::apple_gc
