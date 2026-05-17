#pragma once

#include "core/horizon/kernel/applet_resource.hpp"
#include "core/horizon/kernel/event.hpp"

namespace hydra::horizon::kernel {

class Event;
class Kernel;

#pragma pack(push, 1)
struct AccountHeader {
    u32 magic;
    aligned<u8, 4> unk_x4;
    uuid_t user_id;
    u8 unk_x18[0x70]; // Unused
};
#pragma pack(pop)

class AppletState {
  public:
    AppletState(Kernel& kernel_);
    ~AppletState();

    // Send
    void SendMessage(AppletMessage msg);
    void SetFocusState(AppletFocusState focus_state_);

    void LockExit() { exit_locked = true; }
    void UnlockExit() { exit_locked = false; }

    void PushPreselectedUser(uuid_t user_id);

    // Receive
    AppletMessage ReceiveMessage();
    AppletFocusState GetFocusState() { return focus_state; }
    bool IsExitLocked() { return exit_locked; }
    sized_ptr PopLaunchParameter(const LaunchParameterKind kind);

  private:
    Kernel& kernel;

    std::mutex mutex;

    AppletResourceUserId aruid;

    std::queue<AppletMessage> msg_queue;
    std::atomic<AppletFocusState> focus_state;
    std::atomic<bool> exit_locked{false};
    // TODO: is stack correct?
    std::stack<uuid_t> user_ids;

    // Events
    Event* msg_event;

    // Impl
    void SendMessageImpl(AppletMessage msg);

  public:
    GETTER(aruid, GetAppletResourceUserId);
    GETTER(msg_event, GetMsgEvent);
};

} // namespace hydra::horizon::kernel
