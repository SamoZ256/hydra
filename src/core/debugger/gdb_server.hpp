#pragma once

#include "core/debugger/const.hpp"
#include "core/debugger/target.hpp"

namespace hydra::debugger {

class Debugger;

class GdbServer {
  public:
    explicit GdbServer(Debugger& debugger_);
    ~GdbServer();

  private:
    enum class SessionEnd {
        None,
        Disconnected,
        Detached,
        Killed,
        ProcessExited,
    };

    Debugger& debugger;

    i32 server_socket{-1};
    i32 client_socket{-1};
    std::thread server_thread;
    std::atomic<bool> running{true};
    std::string receive_buffer;
    bool do_ack{true};
    bool awaiting_stop_reply{false};
    SessionEnd session_end{SessionEnd::None};
    thread_id_t crnt_thread_id{invalid<thread_id_t>()};
    std::optional<std::chrono::steady_clock::time_point> stop_deadline;
    std::optional<thread_id_t> interrupted_thread;

    void serverLoop();
    void poll();
    void acceptClient();
    void receiveFromClient();
    void pollStop();
    void finishSession();
    void waitForAnyThread();
    bool waitForAllStopped(i64 timeout_ns);

    bool trySendAll(std::span<const char> bytes);
    void sendPacket(std::string_view data);
    void sendStatus(char status);

    void processPackets();
    void handleCommand(std::string_view command);
    void handleInterrupt();

    // Commands
    void handleVCont(std::string_view command);
    void handleQuery(std::string_view command);
    void handleSetActiveThread(std::string_view command);
    void handleThreadStatus();
    void handleThreadStopInfo(std::string_view command);
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
    void handleMonitorHelp();
    void handleMonitorDisassemble(std::string_view args, bool function);
    void sendConsoleOutput(std::string_view text);
    void sendMonitorError(std::string_view message);
    std::expected<vaddr_t, std::string_view>
    resolveMonitorAddress(std::string_view arg);
    std::optional<vaddr_t> tryGetCurrentPc();

    void resumeTarget();
    void resumeTarget(std::span<const ResumeAction> actions);
    void stepTargetThread(thread_id_t id);
    void replyCurrentStop();
    void beginResume();
    thread_id_t resolveCurrentThreadId();

    // Helpers
    static void setNonBlocking(i32 socket);
    static std::optional<std::string>
    formatReg(const hw::tegra_x1::cpu::ThreadState& state, u32 id);
    static bool tryParseReg(hw::tegra_x1::cpu::ThreadState& state, u32 id,
                            std::string_view hex);
    std::string formatStopReply(thread_id_t id, Signal signal);
    Signal signalForThread(thread_id_t id, Signal fallback) const;
    static Signal signalFromStop(const StopInfo& stop);

    Target& getTarget() const;
    static std::optional<std::string> pageFromBuffer(std::string_view buffer,
                                                     std::string_view page);
};

} // namespace hydra::debugger
