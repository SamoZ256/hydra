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
    enum class SessionEnd {
        None,
        Disconnected,
        Detached,
        Killed,
        ProcessExited,
    };

    struct PendingStop {
        thread_id_t id;
        hw::tegra_x1::cpu::HaltReason reason;
    };

    Debugger& debugger;

    std::mutex mutex;
    std::optional<PendingStop> pending_stop;

    i32 server_socket{-1};
    i32 client_socket{-1};
    std::thread server_thread;
    std::atomic<bool> running{true};
    std::string receive_buffer;
    bool do_ack{true};
    bool awaiting_stop_reply{false};
    SessionEnd session_end{SessionEnd::None};
    thread_id_t crnt_thread_id{invalid<thread_id_t>()};

    void serverLoop();
    void poll();
    void acceptClient();
    void receiveFromClient();
    void flushPendingStop();
    void finishSession();
    void waitForAnyThread();

    bool trySendAll(std::span<const char> bytes);
    void sendPacket(std::string_view data);
    void sendStatus(char status);

    void processPackets();
    void handleCommand(std::string_view command);

    // Commands
    void handleVCont(std::string_view command);
    void handleQuery(std::string_view command);
    void handleSetActiveThread(std::string_view command);
    void handleThreadStatus();
    void handleThreadAlive(std::string_view command);
    void handleDetach();
    void handleKill();
    void handleRegRead(std::string_view command);
    void handleRegReadAll();
    void handleRegWrite(std::string_view command);
    void handleRegWriteAll(std::string_view command);
    void handleMemRead(std::string_view command);
    void handleMemWrite(std::string_view command);
    void handleMemWriteBinary(std::string_view command);
    void handleInsertBreakpoint(std::string_view command);
    void handleRemoveBreakpoint(std::string_view command);

    void handleRcmd(std::string_view cmd);
    void handleGetExecutables();

    void resumeTarget();
    void stepTargetThread(thread_id_t id);
    thread_id_t resolveCurrentThreadId();

    // Helpers
    static void setNonBlocking(i32 socket);
    static std::optional<std::string>
    formatReg(const hw::tegra_x1::cpu::ThreadState& state, u32 id);
    static bool tryParseReg(hw::tegra_x1::cpu::ThreadState& state, u32 id,
                            std::string_view hex);
    std::optional<std::string>
    getThreadStatus(thread_id_t id, Signal signal,
                    i64 timeout_ns = DEFAULT_STOP_TIMEOUT_NS);
    static Signal signalFromHaltReason(hw::tegra_x1::cpu::HaltReason reason);

    Target& getTarget() const;
    static std::optional<std::string> pageFromBuffer(std::string_view buffer,
                                                     std::string_view page);
};

} // namespace hydra::debugger
