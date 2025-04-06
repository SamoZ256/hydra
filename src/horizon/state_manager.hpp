#pragma once

#include "horizon/kernel.hpp"

namespace Hydra::Horizon {

enum class LaunchParameterKind : u32 {
    UserChannel = 1,
    PreselectedUser,
    Unknown0,
};

struct Account {
    u32 magic;
    u8 unk_x4;
    u8 pad[3];
    u128 uid;
    u8 unk_x18[0x70]; // Unused
};

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

    void PushPreselectedUser(u128 account_uid) {
        std::unique_lock<std::mutex> lock(mutex);
        account_uids.push(account_uid);
    }

    // Receive
    AppletMessage ReceiveMessage() {
        std::unique_lock<std::mutex> lock(mutex);
        if (msg_queue.empty()) {
            return AppletMessage::None;
        }

        AppletMessage msg = msg_queue.front();
        msg_queue.pop();

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
            if (account_uids.empty()) {
                LOG_ERROR(Horizon, "No preselected user");
                return {};
            }

            const u128 account_uid = account_uids.top();
            account_uids.pop();

            return {new Account{
                        .magic = 0xc79497ca,
                        .unk_x4 = 1,
                        .uid = account_uid,
                    },
                    sizeof(Account)};
        }
        default:
            LOG_NOT_IMPLEMENTED(Horizon, "Launch parameter {}", kind);
            return {};
        }
    }

    // Getters
    const KernelHandleWithId<SynchronizationHandle>& GetMsgEvent() {
        return msg_event;
    }

  private:
    std::mutex mutex;
    std::queue<AppletMessage> msg_queue;
    AppletFocusState focus_state;
    bool exit_locked{false};

    // TODO: is stack correct?
    std::stack<u128> account_uids;

    // Events
    KernelHandleWithId<SynchronizationHandle> msg_event;
};

} // namespace Hydra::Horizon

ENABLE_ENUM_FORMATTING(Hydra::Horizon::LaunchParameterKind, UserChannel,
                       "user channel", PreselectedUser, "preselected user",
                       Unknown0, "unknown0")
