#pragma once

#include <stack>

#include "core/horizon/kernel/applet_resource.hpp"
#include "core/horizon/kernel/event.hpp"

namespace hydra::horizon::kernel {

class Event;
class Kernel;

#pragma pack(push, 1)
struct AccountHeader {
    u32 magic;
    Aligned<u8, 4> unk_x4;
    uuid_t user_id;
    std::array<u8, 0x70> unk_x18; // Unused
};
#pragma pack(pop)

class AppletState {
  public:
    explicit AppletState(Kernel& kernel_);
    ~AppletState();

    // Send
    void sendMessage(AppletMessage msg);
    void setFocusState(AppletFocusState focus_state_);

    void lockExit() { exit_locked = true; }
    void unlockExit() { exit_locked = false; }

    void pushPreselectedUser(uuid_t user_id);

    // Receive
    AppletMessage receiveMessage();
    AppletFocusState getFocusState() { return focus_state; }
    bool isExitLocked() { return exit_locked; }
    std::vector<u8> popLaunchParameter(const LaunchParameterKind kind);

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
    void sendMessageImpl(AppletMessage msg);

  public:
    GETTER(aruid, getAppletResourceUserId);
    GETTER(msg_event, getMsgEvent);
};

} // namespace hydra::horizon::kernel
