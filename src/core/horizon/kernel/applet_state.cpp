#include "core/horizon/kernel/applet_state.hpp"

#include "core/horizon/kernel/event.hpp"
#include "core/horizon/kernel/kernel.hpp"

namespace hydra::horizon::kernel {

AppletState::AppletState(Kernel& kernel_)
    : kernel{kernel_}, aruid{kernel.AllocateAppletResourceUserId()},
      msg_event{new Event(false, "Message event")} {}

AppletState::~AppletState() {
    msg_event->Release();
    kernel.ReleaseAppletResourceUserId(aruid);
}

void AppletState::SendMessage(AppletMessage msg) {
    std::lock_guard lock(mutex);
    SendMessageImpl(msg);
}

void AppletState::SetFocusState(AppletFocusState focus_state_) {
    {
        std::lock_guard lock(mutex);
        SendMessageImpl(AppletMessage::FocusStateChanged);
        if (focus_state_ == AppletFocusState::InFocus)
            SendMessageImpl(AppletMessage::ChangeIntoForeground);
    }
    focus_state = focus_state_;
}

void AppletState::PushPreselectedUser(uuid_t user_id) {
    std::lock_guard lock(mutex);
    user_ids.push(user_id);
}

AppletMessage AppletState::ReceiveMessage() {
    std::lock_guard lock(mutex);
    if (msg_queue.empty()) {
        return AppletMessage::None;
    }

    AppletMessage msg = msg_queue.front();
    msg_queue.pop();

    // Clear event
    if (msg_queue.empty())
        msg_event->Clear();

    return msg;
}

sized_ptr AppletState::PopLaunchParameter(const LaunchParameterKind kind) {
    std::lock_guard lock(mutex);
    switch (kind) {
    case LaunchParameterKind::PreselectedUser: {
        if (user_ids.empty()) {
            LOG_ERROR(Horizon, "No preselected user");
            return {};
        }

        const uuid_t user_id = user_ids.top();
        user_ids.pop();

        return {new AccountHeader{
            .magic = 0xc79497ca,
            .unk_x4 = 1,
            .user_id = user_id,
        }};
    }
    default:
        LOG_NOT_IMPLEMENTED(Horizon, "Launch parameter {}", kind);
        return {};
    }
}

void AppletState::SendMessageImpl(AppletMessage msg) {
    msg_queue.push(msg);

    // Signal event
    msg_event->Signal();
}

} // namespace hydra::horizon::kernel
