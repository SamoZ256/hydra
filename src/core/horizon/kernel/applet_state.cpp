#include "core/horizon/kernel/applet_state.hpp"

#include "core/horizon/kernel/event.hpp"

namespace hydra::horizon::kernel {

AppletState::AppletState()
    : msg_event{new Event(EventFlags::None, "Message event")} {}
AppletState::~AppletState() { msg_event->Release(); }

void AppletState::SendMessage(AppletMessage msg) {
    std::lock_guard<std::mutex> lock(mutex);
    SendMessageImpl(lock, msg);
}

void AppletState::SetFocusState(AppletFocusState focus_state_) {
    {
        std::lock_guard<std::mutex> lock(mutex);
        SendMessageImpl(lock, AppletMessage::FocusStateChanged);
        if (focus_state_ == AppletFocusState::InFocus)
            SendMessageImpl(lock, AppletMessage::ChangeIntoForeground);
    }
    focus_state = focus_state_;
}

void AppletState::PushPreselectedUser(uuid_t user_id) {
    std::lock_guard<std::mutex> lock(mutex);
    user_ids.push(user_id);
}

AppletMessage AppletState::ReceiveMessage() {
    std::lock_guard<std::mutex> lock(mutex);
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
    std::lock_guard<std::mutex> lock(mutex);
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

void AppletState::SendMessageImpl(std::lock_guard<std::mutex>& lock,
                                  AppletMessage msg) {
    msg_queue.push(msg);

    // Signal event
    msg_event->Signal();
}

} // namespace hydra::horizon::kernel
