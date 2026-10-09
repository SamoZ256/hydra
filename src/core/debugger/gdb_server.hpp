#pragma once

#include "core/debugger/const.hpp"
#include "core/debugger/target.hpp"

namespace hydra::debugger {

class Debugger;

class GdbServer {
  public:
    explicit GdbServer(Debugger& debugger_);
    ~GdbServer();

    void notifyHalted(thread_id_t id, hw::tegra_x1::cpu::HaltReason reason);

  private:
    Debugger& debugger;

    std::mutex mutex;

    i32 server_socket;
    i32 client_socket{-1};
    std::thread server_thread;
    std::atomic<bool> running{true};
    std::string receive_buffer;
    bool do_ack{true};

    std::atomic<thread_id_t> crnt_thread_id{invalid<thread_id_t>()};

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
    void handleRegReadAll();
    void handleRegWrite(std::string_view command);
    void handleMemRead(std::string_view command);
    void handleMemWrite(std::string_view command);
    void handleInsertBreakpoint(std::string_view command);
    void handleRemoveBreakpoint(std::string_view command);

    void handleRcmd(std::string_view cmd);
    void handleGetExecutables();

    // Helpers
    static void setNonBlocking(i32 socket);
    static std::optional<std::string>
    formatReg(const hw::tegra_x1::cpu::ThreadState& state, u32 id);
    static bool tryParseReg(hw::tegra_x1::cpu::ThreadState& state, u32 id,
                            std::string_view hex);
    std::optional<std::string> getThreadStatus(thread_id_t id, Signal signal);
    static Signal signalFromHaltReason(hw::tegra_x1::cpu::HaltReason reason);

    Target& getTarget() const;
    static std::string pageFromBuffer(std::string_view buffer,
                                      std::string_view page);
};

} // namespace hydra::debugger
