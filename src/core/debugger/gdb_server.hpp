#pragma once

#include "core/debugger/const.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::kernel {
class GuestThread;
} // namespace hydra::horizon::kernel

namespace hydra::debugger {

class Thread;
class Debugger;

class GdbServer {
  public:
    GdbServer(System& system_, Debugger& debugger_);
    ~GdbServer();

    void registerThread(Thread& thread);

    void notifySupervisorPaused(horizon::kernel::GuestThread* thread,
                                Signal signal);
    void breakpointHit(horizon::kernel::GuestThread* thread);

  private:
    System& system;
    Debugger& debugger;

    std::mutex mutex;

    i32 server_socket;
    i32 client_socket{-1};
    std::thread server_thread;
    std::atomic<bool> running{true};
    std::string receive_buffer;
    bool do_ack{true};

    horizon::kernel::GuestThread* crnt_thread;
    std::vector<vaddr_t> breakpoint_addresses;
    std::map<vaddr_t, u32> replaced_instructions;
    std::atomic<bool> breakpoint_hit{false};
    horizon::kernel::GuestThread* breakpoint_thread{nullptr};

    void closeClientSocket();

    void serverLoop();
    void poll();

    void sendPacket(std::string_view data) const;
    void sendStatus(char status) const;

    void processPackets();
    void handleCommand(std::string_view command);

    // Commands
    void handleVCont(std::string_view command);
    void handleQuery(std::string_view command);
    void handleSetActiveThread(std::string_view command);
    void handleThreadStatus();
    void handleRegRead(std::string_view command);
    void handleMemRead(std::string_view command);
    void handleInsertBreakpoint(std::string_view command);
    void handleRemoveBreakpoint(std::string_view command);

    void handleRcmd(std::string_view cmd);
    void handleGetExecutables();

    // Helpers
    static void setNonBlocking(i32 socket);
    std::string readReg(u32 id);
    static std::string getThreadStatus(horizon::kernel::GuestThread* thread,
                                       Signal signal);
    static std::string pageFromBuffer(std::string_view buffer,
                                      std::string_view page);

    void notifySupervisorPausedImpl(horizon::kernel::GuestThread* thread,
                                    Signal signal);
    void notifyMemoryChanged(ztd::Range<vaddr_t> mem_range);
};

} // namespace hydra::debugger
