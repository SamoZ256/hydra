#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/kernel/kernel.hpp"

namespace hydra::horizon {

#pragma pack(push, 1)
struct AccountHeader {
    u32 magic;
    aligned<u8, 4> unk_x4;
    uuid_t user_id;
    u8 unk_x18[0x70]; // Unused
};
#pragma pack(pop)

class StateManager {
  public:
    static StateManager& GetInstance();

    StateManager();
    ~StateManager();

    // Send
    void SendMessage(AppletMessage msg) {
        std::unique_lock<std::mutex> lock(mutex);
        msg_queue.push(msg);

        // Signal event
        msg_event.handle->Signal();
    }

    void SetFocusState(AppletFocusState focus_state_) {
        SendMessage(AppletMessage::FocusStateChanged);
        if (focus_state_ == AppletFocusState::InFocus)
            SendMessage(AppletMessage::ChangeIntoForeground);
        // TODO: lock
        focus_state = focus_state_;
    }

    void LockExit() {
        std::unique_lock<std::mutex> lock(mutex);
        exit_locked = true;
    }

    void UnlockExit() {
        std::unique_lock<std::mutex> lock(mutex);
        exit_locked = false;
    }

    void PushPreselectedUser(uuid_t user_id) {
        std::unique_lock<std::mutex> lock(mutex);
        user_ids.push(user_id);
    }

    // Receive
    AppletMessage ReceiveMessage() {
        std::unique_lock<std::mutex> lock(mutex);
        if (msg_queue.empty()) {
            return AppletMessage::None;
        }

        AppletMessage msg = msg_queue.front();
        msg_queue.pop();

        // Clear event
        if (msg_queue.empty())
            msg_event.handle->Clear();

        return msg;
    }

    AppletFocusState GetFocusState() {
        std::unique_lock<std::mutex> lock(mutex);
        const auto f_state = focus_state;

        return f_state;
    }

    bool IsExitLocked() {
        std::unique_lock<std::mutex> lock(mutex);
        return exit_locked;
    }

    sized_ptr PopLaunchParameter(const LaunchParameterKind kind) {
        std::unique_lock<std::mutex> lock(mutex);
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
                    },
                    sizeof(AccountHeader)};
        }
        default:
            LOG_NOT_IMPLEMENTED(Horizon, "Launch parameter {}", kind);
            return {};
        }
    }

    // Getters
    const kernel::HandleWithId<kernel::Event>& GetMsgEvent() {
        return msg_event;
    }

  private:
    std::mutex mutex;
    std::queue<AppletMessage> msg_queue;
    AppletFocusState focus_state;
    bool exit_locked{false};

    // TODO: is stack correct?
    std::stack<uuid_t> user_ids;

    // Events
    kernel::HandleWithId<kernel::Event> msg_event;
};

} // namespace hydra::horizon
