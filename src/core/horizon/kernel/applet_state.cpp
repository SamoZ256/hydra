#include "core/horizon/kernel/applet_state.hpp"

#include "core/horizon/kernel/event.hpp"
#include "core/horizon/kernel/kernel.hpp"

namespace hydra::horizon::kernel {

AppletState::AppletState(Kernel& kernel_)
    : kernel{kernel_}, aruid{kernel.allocateAppletResourceUserId()},
      msg_event{new Event(false, "Message event")} {}

AppletState::~AppletState() {
    msg_event->release();
    kernel.releaseAppletResourceUserId(aruid);
}

void AppletState::sendMessage(AppletMessage msg) {
    std::scoped_lock lock(mutex);
    sendMessageImpl(msg);
}

void AppletState::setFocusState(AppletFocusState focus_state_) {
    {
        std::scoped_lock lock(mutex);
        sendMessageImpl(AppletMessage::FocusStateChanged);
        if (focus_state_ == AppletFocusState::InFocus)
            sendMessageImpl(AppletMessage::ChangeIntoForeground);
    }
    focus_state = focus_state_;
}

void AppletState::pushPreselectedUser(uuid_t user_id) {
    std::scoped_lock lock(mutex);
    user_ids.push(user_id);
}

AppletMessage AppletState::receiveMessage() {
    std::scoped_lock lock(mutex);
    if (msg_queue.empty()) {
        return AppletMessage::None;
    }

    AppletMessage msg = msg_queue.front();
    msg_queue.pop();

    // Clear event
    if (msg_queue.empty())
        msg_event->clear();

    return msg;
}

std::vector<u8>
AppletState::popLaunchParameter(const LaunchParameterKind kind) {
    std::scoped_lock lock(mutex);
    // NOLINTNEXTLINE(readability-trivial-switch)
    switch (kind) {
    case LaunchParameterKind::PreselectedUser: {
        if (user_ids.empty()) {
            LOG_ERROR(Horizon, "No preselected user");
            return {};
        }

        const uuid_t user_id = user_ids.top();
        user_ids.pop();

        AccountHeader res{
            .magic = 0xc79497ca,
            .unk_x4 = 1,
            .user_id = user_id,
        };

        std::vector<u8> bytes(sizeof(AccountHeader));
        std::memcpy(bytes.data(), &res, sizeof(AccountHeader));
        return bytes;
    }
    default:
        LOG_NOT_IMPLEMENTED(Horizon, "Launch parameter {}", kind);
        return {};
    }
}

void AppletState::sendMessageImpl(AppletMessage msg) {
    msg_queue.push(msg);

    // Signal event
    msg_event->signal();
}

} // namespace hydra::horizon::kernel
