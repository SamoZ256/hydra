#include "core/debugger/gdb_server.hpp"

#include <cerrno>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include <nx2elf.h>
#pragma GCC diagnostic pop

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/filesystem/file.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

namespace hydra::debugger {

namespace {

constexpr char GDB_START = '$';
constexpr char GDB_END = '#';
constexpr char GDB_ACK = '+';
constexpr char GDB_NACK = '-';

constexpr char GDB_INT3 = 0x03;

constexpr char GDB_OK[] = "OK";
constexpr char GDB_ERROR[] = "E01";
constexpr char GDB_EMPTY[] = "";

constexpr usize MAX_PACKET_SIZE = 0x4000;
constexpr usize MAX_RECEIVE_BUFFER_SIZE = 2 * MAX_PACKET_SIZE + 64;
constexpr usize MAX_LOGGED_PACKET_SIZE = 128;
constexpr i32 SEND_POLL_TIMEOUT_MS = 1000;
constexpr i32 SERVER_POLL_INTERVAL_MS = 10;
constexpr i64 INITIAL_STOP_TIMEOUT_NS = 3000ll * 1000ll * 1000ll;
constexpr i64 ALL_STOP_TIMEOUT_NS = 2000ll * 1000ll * 1000ll;
constexpr i32 STOP_POLL_INTERVAL_MS = 1;
constexpr std::string_view HOST_TRIPLE = "aarch64-unknown-none";

[[maybe_unused]] constexpr u32 FP_REGISTER = 29;
constexpr u32 LR_REGISTER = 30;
constexpr u32 SP_REGISTER = 31;
constexpr u32 PC_REGISTER = 32;
constexpr u32 PSTATE_REGISTER = 33;
constexpr u32 Q0_REGISTER = 34;
constexpr u32 FPSR_REGISTER = 66;
constexpr u32 FPCR_REGISTER = 67;
constexpr usize REG_FILE_HEX_SIZE = 1576;

enum class BreakpointType {
    Software = 0,
    Hardware = 1,
    WriteWatch = 2,
    ReadWatch = 3,
    AccessWatch = 4,
};

constexpr usize regHexWidth(u32 id) {
    if (id == PSTATE_REGISTER || id == FPSR_REGISTER || id == FPCR_REGISTER)
        return sizeof(u32) * 2;
    if (id >= Q0_REGISTER && id < FPSR_REGISTER)
        return sizeof(u128) * 2;
    return sizeof(u64) * 2;
}

u8 packetChecksum(std::string_view data) {
    u8 checksum = 0;
    for (char c : data)
        checksum += static_cast<u8>(c);

    return checksum;
}

std::string_view loggedPacket(std::string_view packet) {
    if (!packet.empty() && packet[0] == 'X')
        return packet.substr(0, packet.find(':'));

    return packet.substr(0, MAX_LOGGED_PACKET_SIZE);
}

constexpr usize MAX_CONSOLE_CHUNK_SIZE = 0x1000;

std::string_view nextWord(std::string_view& args) {
    const auto begin = args.find_first_not_of(' ');
    if (begin == std::string_view::npos) {
        args = {};
        return {};
    }

    args.remove_prefix(begin);
    const auto end = std::min(args.find(' '), args.size());
    const auto word = args.substr(0, end);
    args.remove_prefix(end);
    return word;
}

std::optional<u64> parseMonitorNumber(std::string_view arg, bool hex) {
    if (arg.starts_with("0x")) {
        arg.remove_prefix(2);
        hex = true;
    }

    if (arg.empty())
        return std::nullopt;

    ZTD_ASSIGN_OR(const auto value, fromChars<u64>(arg, hex ? 16 : 10),
                  { return std::nullopt; });
    return value;
}

std::string hexEncode(std::string_view str) {
    std::string hex;
    hex.reserve(str.size() * 2);
    for (const char c : str)
        hex += fmt::format("{:02x}", static_cast<u8>(c));

    return hex;
}

template <typename T>
std::string numberToHex(T value) {
    const auto ptr = reinterpret_cast<const u8*>(&value);
    std::string hex;
    hex.reserve(sizeof(T) * 2);
    for (usize i = 0; i < sizeof(T); ++i)
        hex += fmt::format("{:02x}", ptr[i]);

    return hex;
}

template <typename T>
std::expected<T, std::errc> hexToNumber(std::string_view hex) {
    if (hex.size() < sizeof(T) * 2)
        return std::unexpected(std::errc::invalid_argument);

    T value = 0;
    for (usize i = 0; i < sizeof(T); ++i) {
        ZTD_ASSIGN_OR_RETURN_ERROR(const auto crnt,
                                   fromChars<T>(hex.substr(i * 2, 2), 16));
        value |= crnt << (i * 8);
    }

    return value;
}

struct MemoryRequest {
    vaddr_t addr;
    u64 size;
    std::string_view payload;
};

struct BreakpointRequest {
    BreakpointType type;
    vaddr_t addr;
    u64 size;
};

std::optional<MemoryRequest> parseMemoryRequest(std::string_view command) {
    const auto comma_pos = command.find(',');
    if (comma_pos == std::string_view::npos)
        return std::nullopt;

    auto size_end = command.find(':', comma_pos);
    const auto payload = size_end == std::string_view::npos
                             ? std::string_view()
                             : command.substr(size_end + 1);
    if (size_end == std::string_view::npos)
        size_end = command.size();

    ZTD_ASSIGN_OR_RETURN_VALUE(const auto addr,
                               fromChars<u64>(command.substr(0, comma_pos), 16),
                               std::nullopt);
    ZTD_ASSIGN_OR_RETURN_VALUE(
        const auto size,
        fromChars<u64>(command.substr(comma_pos + 1, size_end - comma_pos - 1),
                       16),
        std::nullopt);
    if (size > MAX_PACKET_SIZE)
        return std::nullopt;

    return MemoryRequest{.addr = addr, .size = size, .payload = payload};
}

std::optional<BreakpointRequest>
parseBreakpointRequest(std::string_view command) {
    const auto first = command.find(',');
    if (first == std::string_view::npos)
        return std::nullopt;

    const auto second = command.find(',', first + 1);
    if (second == std::string_view::npos)
        return std::nullopt;

    ZTD_ASSIGN_OR_RETURN_VALUE(const auto type,
                               fromChars<u8>(command.substr(0, first), 16),
                               std::nullopt);
    ZTD_ASSIGN_OR_RETURN_VALUE(
        const auto addr,
        fromChars<u64>(command.substr(first + 1, second - first - 1), 16),
        std::nullopt);
    ZTD_ASSIGN_OR_RETURN_VALUE(const auto size,
                               fromChars<u64>(command.substr(second + 1), 16),
                               std::nullopt);
    return BreakpointRequest{
        .type = static_cast<BreakpointType>(type), .addr = addr, .size = size};
}

std::optional<hw::tegra_x1::cpu::Watchpoint>
toWatchpoint(const BreakpointRequest& request) {
    hw::tegra_x1::cpu::WatchpointType type;
    switch (request.type) {
    case BreakpointType::WriteWatch:
        type = hw::tegra_x1::cpu::WatchpointType::Write;
        break;
    case BreakpointType::ReadWatch:
        type = hw::tegra_x1::cpu::WatchpointType::Read;
        break;
    case BreakpointType::AccessWatch:
        type = hw::tegra_x1::cpu::WatchpointType::Access;
        break;
    default:
        return std::nullopt;
    }

    return hw::tegra_x1::cpu::Watchpoint{
        .addr = request.addr, .size = request.size, .type = type};
}

std::string_view watchpointKey(hw::tegra_x1::cpu::WatchpointType type) {
    switch (type) {
    case hw::tegra_x1::cpu::WatchpointType::Write:
        return "watch";
    case hw::tegra_x1::cpu::WatchpointType::Read:
        return "rwatch";
    case hw::tegra_x1::cpu::WatchpointType::Access:
        return "awatch";
    }
}

std::optional<std::vector<u8>> unescapeBinary(std::string_view in,
                                              usize expected_size) {
    std::vector<u8> out;
    out.reserve(expected_size);
    for (usize i = 0; i < in.size(); i++) {
        auto c = static_cast<u8>(in[i]);
        if (c == '}') {
            if (++i >= in.size())
                return std::nullopt;

            c = static_cast<u8>(in[i]) ^ 0x20;
        }

        out.push_back(c);
    }

    if (out.size() != expected_size)
        return std::nullopt;

    return out;
}

std::string_view getTargetXmlAarch64() {
    return R"(<?xml version="1.0"?>
    <!DOCTYPE target SYSTEM "gdb-target.dtd">
    <target version="1.0">
      <architecture>aarch64</architecture>
      <feature name="org.gnu.gdb.aarch64.core">
        <reg name="x0" bitsize="64"/>
        <reg name="x1" bitsize="64"/>
        <reg name="x2" bitsize="64"/>
        <reg name="x3" bitsize="64"/>
        <reg name="x4" bitsize="64"/>
        <reg name="x5" bitsize="64"/>
        <reg name="x6" bitsize="64"/>
        <reg name="x7" bitsize="64"/>
        <reg name="x8" bitsize="64"/>
        <reg name="x9" bitsize="64"/>
        <reg name="x10" bitsize="64"/>
        <reg name="x11" bitsize="64"/>
        <reg name="x12" bitsize="64"/>
        <reg name="x13" bitsize="64"/>
        <reg name="x14" bitsize="64"/>
        <reg name="x15" bitsize="64"/>
        <reg name="x16" bitsize="64"/>
        <reg name="x17" bitsize="64"/>
        <reg name="x18" bitsize="64"/>
        <reg name="x19" bitsize="64"/>
        <reg name="x20" bitsize="64"/>
        <reg name="x21" bitsize="64"/>
        <reg name="x22" bitsize="64"/>
        <reg name="x23" bitsize="64"/>
        <reg name="x24" bitsize="64"/>
        <reg name="x25" bitsize="64"/>
        <reg name="x26" bitsize="64"/>
        <reg name="x27" bitsize="64"/>
        <reg name="x28" bitsize="64"/>
        <reg name="x29" bitsize="64"/>
        <reg name="x30" bitsize="64"/>
        <reg name="sp" bitsize="64" type="data_ptr"/>
        <reg name="pc" bitsize="64" type="code_ptr"/>
        <flags id="cpsr_flags" size="4">
          <field name="SP" start="0" end="0"/>
          <field name="" start="1" end="1"/>
          <field name="EL" start="2" end="3"/>
          <field name="nRW" start="4" end="4"/>
          <field name="" start="5" end="5"/>
          <field name="F" start="6" end="6"/>
          <field name="I" start="7" end="7"/>
          <field name="A" start="8" end="8"/>
          <field name="D" start="9" end="9"/>
          <field name="IL" start="20" end="20"/>
          <field name="SS" start="21" end="21"/>
          <field name="V" start="28" end="28"/>
          <field name="C" start="29" end="29"/>
          <field name="Z" start="30" end="30"/>
          <field name="N" start="31" end="31"/>
        </flags>
        <reg name="cpsr" bitsize="32" type="cpsr_flags"/>
      </feature>
      <feature name="org.gnu.gdb.aarch64.fpu">
        <vector id="v2d" type="ieee_double" count="2"/>
        <vector id="v2u" type="uint64" count="2"/>
        <vector id="v2i" type="int64" count="2"/>
        <vector id="v4f" type="ieee_single" count="4"/>
        <vector id="v4u" type="uint32" count="4"/>
        <vector id="v4i" type="int32" count="4"/>
        <vector id="v8u" type="uint16" count="8"/>
        <vector id="v8i" type="int16" count="8"/>
        <vector id="v16u" type="uint8" count="16"/>
        <vector id="v16i" type="int8" count="16"/>
        <vector id="v1u" type="uint128" count="1"/>
        <vector id="v1i" type="int128" count="1"/>
        <union id="vnd">
          <field name="f" type="v2d"/>
          <field name="u" type="v2u"/>
          <field name="s" type="v2i"/>
        </union>
        <union id="vns">
          <field name="f" type="v4f"/>
          <field name="u" type="v4u"/>
          <field name="s" type="v4i"/>
        </union>
        <union id="vnh">
          <field name="u" type="v8u"/>
          <field name="s" type="v8i"/>
        </union>
        <union id="vnb">
          <field name="u" type="v16u"/>
          <field name="s" type="v16i"/>
        </union>
        <union id="vnq">
          <field name="u" type="v1u"/>
          <field name="s" type="v1i"/>
        </union>
        <union id="aarch64v">
          <field name="d" type="vnd"/>
          <field name="s" type="vns"/>
          <field name="h" type="vnh"/>
          <field name="b" type="vnb"/>
          <field name="q" type="vnq"/>
        </union>
        <reg name="v0" bitsize="128" type="aarch64v" regnum="34"/>
        <reg name="v1" bitsize="128" type="aarch64v" />
        <reg name="v2" bitsize="128" type="aarch64v" />
        <reg name="v3" bitsize="128" type="aarch64v" />
        <reg name="v4" bitsize="128" type="aarch64v" />
        <reg name="v5" bitsize="128" type="aarch64v" />
        <reg name="v6" bitsize="128" type="aarch64v" />
        <reg name="v7" bitsize="128" type="aarch64v" />
        <reg name="v8" bitsize="128" type="aarch64v" />
        <reg name="v9" bitsize="128" type="aarch64v" />
        <reg name="v10" bitsize="128" type="aarch64v"/>
        <reg name="v11" bitsize="128" type="aarch64v"/>
        <reg name="v12" bitsize="128" type="aarch64v"/>
        <reg name="v13" bitsize="128" type="aarch64v"/>
        <reg name="v14" bitsize="128" type="aarch64v"/>
        <reg name="v15" bitsize="128" type="aarch64v"/>
        <reg name="v16" bitsize="128" type="aarch64v"/>
        <reg name="v17" bitsize="128" type="aarch64v"/>
        <reg name="v18" bitsize="128" type="aarch64v"/>
        <reg name="v19" bitsize="128" type="aarch64v"/>
        <reg name="v20" bitsize="128" type="aarch64v"/>
        <reg name="v21" bitsize="128" type="aarch64v"/>
        <reg name="v22" bitsize="128" type="aarch64v"/>
        <reg name="v23" bitsize="128" type="aarch64v"/>
        <reg name="v24" bitsize="128" type="aarch64v"/>
        <reg name="v25" bitsize="128" type="aarch64v"/>
        <reg name="v26" bitsize="128" type="aarch64v"/>
        <reg name="v27" bitsize="128" type="aarch64v"/>
        <reg name="v28" bitsize="128" type="aarch64v"/>
        <reg name="v29" bitsize="128" type="aarch64v"/>
        <reg name="v30" bitsize="128" type="aarch64v"/>
        <reg name="v31" bitsize="128" type="aarch64v"/>
        <reg name="fpsr" bitsize="32"/>
        <reg name="fpcr" bitsize="32"/>
      </feature>
    </target>)";
}

} // namespace

} // namespace hydra::debugger

ENABLE_ENUM_FORMATTING(hydra::debugger::BreakpointType, Software, "software",
                       Hardware, "hardware", WriteWatch, "write watch",
                       ReadWatch, "read watch", AccessWatch, "access watch")

namespace hydra::debugger {

GdbServer::GdbServer(Debugger& debugger_) : debugger{debugger_} {
    const u16 port = CONFIG_INSTANCE.getGdbPort();

    // Create the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        LOG_ERROR(Debugger, "Failed to create GDB socket");
        return;
    }

    // Set the socket to reuse address
    i32 opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<char*>(&opt), sizeof(opt));

    // Bind the socket to the address
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(server_socket, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) ==
        -1) {
        LOG_ERROR(Debugger, "Failed to bind GDB socket");
        close(server_socket);
        server_socket = -1;
        return;
    }

    // Start listening
    if (listen(server_socket, 1) == -1) {
        LOG_ERROR(Debugger, "Failed to listen on GDB socket");
        close(server_socket);
        server_socket = -1;
        return;
    }

    // Set the socket to non-blocking mode
    setNonBlocking(server_socket);

    // Create server thread
    server_thread = std::thread(&GdbServer::serverLoop, this);

    LOG_INFO(Debugger, "GDB server started on port {}", port);
}

GdbServer::~GdbServer() {
    running = false;
    if (server_thread.joinable())
        server_thread.join();
    if (client_socket != -1)
        close(client_socket);
    if (server_socket != -1)
        close(server_socket);
}

void GdbServer::serverLoop() {
    GET_CURRENT_PROCESS_DEBUGGER().registerThisThread("GDB server");
    while (running) {
        poll();
        std::this_thread::sleep_for(
            std::chrono::milliseconds(SERVER_POLL_INTERVAL_MS));
    }
    GET_CURRENT_PROCESS_DEBUGGER().unregisterThisThread();
}

void GdbServer::poll() {
    if (client_socket == -1) {
        acceptClient();
        return;
    }

    receiveFromClient();
    if (session_end == SessionEnd::None)
        pollStop();
    if (session_end != SessionEnd::None)
        finishSession();
}

void GdbServer::acceptClient() {
    sockaddr_in client_addr{};
    socklen_t addr_len = sizeof(client_addr);
    const i32 new_client = accept(
        server_socket, reinterpret_cast<sockaddr*>(&client_addr), &addr_len);
    if (new_client == -1)
        return;

    client_socket = new_client;
    setNonBlocking(client_socket);
    i32 opt = 1;
    setsockopt(client_socket, SOL_SOCKET, SO_NOSIGPIPE,
               reinterpret_cast<char*>(&opt), sizeof(opt));

    stop_deadline.reset();
    interrupted_thread.reset();
    getTarget().pauseAll();
    waitForAnyThread();
    crnt_thread_id = resolveCurrentThreadId();
    LOG_INFO(Debugger, "GDB client connected");
}

void GdbServer::waitForAnyThread() {
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::nanoseconds(INITIAL_STOP_TIMEOUT_NS);
    while (getTarget().getThreadIds().empty() &&
           getTarget().isProcessRunning() &&
           std::chrono::steady_clock::now() < deadline)
        std::this_thread::sleep_for(
            std::chrono::milliseconds(SERVER_POLL_INTERVAL_MS));
}

void GdbServer::receiveFromClient() {
    char buffer[1024];
    while (true) {
        const auto bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_read > 0) {
            receive_buffer.append(buffer, static_cast<usize>(bytes_read));
            if (receive_buffer.size() > MAX_RECEIVE_BUFFER_SIZE)
                break;
            continue;
        }

        if (bytes_read == 0 ||
            (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR)) {
            session_end = SessionEnd::Disconnected;
            return;
        }

        if (errno != EINTR)
            break;
    }

    if (!receive_buffer.empty())
        processPackets();
}

bool GdbServer::waitForAllStopped(i64 timeout_ns) {
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::nanoseconds(timeout_ns);
    while (!getTarget().isAllStopped()) {
        if (std::chrono::steady_clock::now() >= deadline)
            return false;

        std::this_thread::sleep_for(
            std::chrono::milliseconds(STOP_POLL_INTERVAL_MS));
    }

    return true;
}

void GdbServer::pollStop() {
    if (!awaiting_stop_reply)
        return;

    auto& target = getTarget();
    if (!target.isProcessRunning()) {
        sendPacket("W00");
        awaiting_stop_reply = false;
        session_end = SessionEnd::ProcessExited;
        return;
    }

    if (!target.isStopping())
        return;

    const auto now = std::chrono::steady_clock::now();
    if (!stop_deadline)
        stop_deadline = now + std::chrono::nanoseconds(ALL_STOP_TIMEOUT_NS);

    if (!target.isAllStopped()) {
        if (now < stop_deadline.value())
            return;

        LOG_WARN(Debugger, "Reporting a stop before all threads parked");
    }

    const auto snapshots = target.getThreadSnapshots();
    if (snapshots.empty())
        return;

    const ThreadSnapshot* chosen = nullptr;
    for (const auto& snapshot : snapshots) {
        if (!snapshot.resumed || !snapshot.stop)
            continue;

        if (chosen == nullptr) {
            chosen = &snapshot;
            continue;
        }

        const bool step = snapshot.stop->reason ==
                          hw::tegra_x1::cpu::HaltReason::StepComplete;
        const bool chosen_step =
            chosen->stop->reason == hw::tegra_x1::cpu::HaltReason::StepComplete;
        if (step != chosen_step) {
            if (step)
                chosen = &snapshot;
            continue;
        }

        if (snapshot.stop->sequence < chosen->stop->sequence)
            chosen = &snapshot;
    }

    thread_id_t id;
    Signal signal;
    if (chosen != nullptr) {
        id = chosen->id;
        signal = signalFromStop(chosen->stop.value());
    } else {
        const auto resumed =
            std::ranges::find_if(snapshots, [this](const auto& snapshot) {
                return snapshot.resumed && snapshot.id == crnt_thread_id;
            });
        if (resumed != snapshots.end()) {
            id = resumed->id;
        } else {
            const auto first_resumed =
                std::ranges::find_if(snapshots, [](const auto& snapshot) {
                    return snapshot.resumed;
                });
            id = first_resumed != snapshots.end() ? first_resumed->id
                                                  : snapshots.front().id;
        }
        signal = Signal::SigInt;
        interrupted_thread = id;
    }

    sendPacket(formatStopReply(id, signal));
    crnt_thread_id = id;
    awaiting_stop_reply = false;
    stop_deadline.reset();
}

void GdbServer::finishSession() {
    const auto end = session_end;
    session_end = SessionEnd::None;

    if (client_socket != -1) {
        close(client_socket);
        client_socket = -1;
    }

    receive_buffer.clear();
    do_ack = true;
    awaiting_stop_reply = false;
    crnt_thread_id = invalid<thread_id_t>();
    stop_deadline.reset();
    interrupted_thread.reset();

    switch (end) {
    case SessionEnd::Disconnected:
    case SessionEnd::Detached:
        getTarget().removeAllBreakpoints();
        getTarget().removeAllWatchpoints();
        getTarget().resumeAll();
        break;
    case SessionEnd::Killed:
        getTarget().stop();
        break;
    case SessionEnd::ProcessExited:
    case SessionEnd::None:
        break;
    }

    LOG_INFO(Debugger, "GDB client disconnected");
}

bool GdbServer::trySendAll(std::span<const char> bytes) {
    if (client_socket == -1)
        return false;

    usize offset = 0;
    while (offset < bytes.size()) {
        const auto sent = send(client_socket, bytes.data() + offset,
                               bytes.size() - offset, 0);
        if (sent >= 0) {
            offset += static_cast<usize>(sent);
            continue;
        }

        if (errno == EINTR)
            continue;

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            pollfd pfd{.fd = client_socket, .events = POLLOUT, .revents = 0};
            if (::poll(&pfd, 1, SEND_POLL_TIMEOUT_MS) > 0)
                continue;
        }

        LOG_WARN(Debugger, "Failed to send to the GDB client");
        session_end = SessionEnd::Disconnected;
        return false;
    }

    return true;
}

void GdbServer::sendPacket(std::string_view data) {
    const std::string packet =
        fmt::format("${}#{:02x}", data, packetChecksum(data));
    LOG_DEBUG(Debugger, "SENDING: {}", loggedPacket(packet));
    trySendAll(std::span<const char>(packet.data(), packet.size()));
}

void GdbServer::sendStatus(char status) {
    if (!do_ack)
        return;

    trySendAll(std::span<const char>(&status, 1));
}

void GdbServer::processPackets() {
    usize str_start = 0;
    while (session_end == SessionEnd::None) {
        const auto start = receive_buffer.find(GDB_START, str_start);
        const auto scan_end =
            start == std::string::npos ? receive_buffer.size() : start;
        if (std::string_view(receive_buffer)
                .substr(str_start, scan_end - str_start)
                .contains(GDB_INT3))
            handleInterrupt();

        if (start == std::string::npos) {
            str_start = receive_buffer.size();
            break;
        }

        const auto end = receive_buffer.find(GDB_END, start);
        if (end == std::string::npos || end + 2 >= receive_buffer.size()) {
            str_start = start;
            break;
        }

        const std::string command =
            receive_buffer.substr(start + 1, end - start - 1);
        const auto checksum = fromChars<u8>(
            std::string_view(receive_buffer).substr(end + 1, 2), 16);
        str_start = end + 3;

        if (!checksum || checksum.value() != packetChecksum(command)) {
            LOG_WARN(Debugger, "GDB packet checksum mismatch");
            sendStatus(GDB_NACK);
            continue;
        }

        sendStatus(GDB_ACK);
        if (!command.empty())
            handleCommand(command);
    }

    receive_buffer.erase(0, str_start);
    if (receive_buffer.size() > MAX_RECEIVE_BUFFER_SIZE) {
        LOG_WARN(Debugger, "Dropping oversized GDB packet");
        receive_buffer.clear();
    }
}

void GdbServer::handleCommand(std::string_view command) {
    LOG_DEBUG(Debugger, "COMMAND: {}", loggedPacket(command));

    if (command.starts_with("vCont")) {
        handleVCont(command.substr(5));
        return;
    }

    const auto body = command.substr(1);
    switch (command[0]) {
    case 'Q':
    case 'q':
        handleQuery(body);
        break;
    case 'k':
        handleKill();
        break;
    case 'D':
        handleDetach();
        break;
    case 'H':
        handleSetActiveThread(body);
        break;
    case 'T':
        handleThreadAlive(body);
        break;
    case '?':
        handleThreadStatus();
        break;
    case 'p':
        handleRegRead(body);
        break;
    case 'g':
        handleRegReadAll();
        break;
    case 'P':
        handleRegWrite(body);
        break;
    case 'G':
        handleRegWriteAll(body);
        break;
    case 'm':
        handleMemRead(body);
        break;
    case 'M':
        handleMemWrite(body);
        break;
    case 'X':
        handleMemWriteBinary(body);
        break;
    case 'c':
    case 'C':
        resumeTarget();
        break;
    case 's':
    case 'S':
        stepTargetThread(resolveCurrentThreadId());
        break;
    case 'Z':
        handleInsertBreakpoint(body);
        break;
    case 'z':
        handleRemoveBreakpoint(body);
        break;
    default:
        LOG_WARN(Debugger, "Unhandled GDB command: {}", loggedPacket(command));
        sendPacket(GDB_EMPTY);
        break;
    }
}

void GdbServer::handleInterrupt() {
    if (!awaiting_stop_reply)
        return;

    getTarget().pauseAll();
}

void GdbServer::handleVCont(std::string_view command) {
    if (command == "?") {
        sendPacket("vCont;c;C;s;S");
        return;
    }

    if (command.empty() || command[0] != ';') {
        sendPacket(GDB_ERROR);
        return;
    }

    struct Action {
        ResumeKind kind;
        std::optional<thread_id_t> thread_id;
    };
    std::vector<Action> parsed;

    usize pos = 1;
    while (pos <= command.size()) {
        const auto next_pos = command.find(';', pos);
        const auto entry = command.substr(
            pos, next_pos == std::string_view::npos ? std::string_view::npos
                                                    : next_pos - pos);
        pos = next_pos == std::string_view::npos ? command.size() + 1
                                                 : next_pos + 1;
        if (entry.empty()) {
            sendPacket(GDB_ERROR);
            return;
        }

        ResumeKind kind;
        bool has_signal = false;
        switch (entry[0]) {
        case 'c':
            kind = ResumeKind::Continue;
            break;
        case 'C':
            kind = ResumeKind::Continue;
            has_signal = true;
            break;
        case 's':
            kind = ResumeKind::Step;
            break;
        case 'S':
            kind = ResumeKind::Step;
            has_signal = true;
            break;
        default:
            sendPacket(GDB_ERROR);
            return;
        }

        auto rest = entry.substr(1);
        if (has_signal) {
            if (rest.size() < 2 || !fromChars<u8>(rest.substr(0, 2), 16)) {
                sendPacket(GDB_ERROR);
                return;
            }

            rest = rest.substr(2);
        }

        std::optional<thread_id_t> thread_id;
        if (!rest.empty()) {
            if (rest[0] != ':') {
                sendPacket(GDB_ERROR);
                return;
            }

            auto tid = rest.substr(1);
            if (tid.starts_with('p')) {
                const auto dot = tid.find('.');
                if (dot == std::string_view::npos) {
                    sendPacket(GDB_ERROR);
                    return;
                }

                tid = tid.substr(dot + 1);
            }

            ZTD_ASSIGN_OR(const auto value, fromChars<i64>(tid, 16), {
                sendPacket(GDB_ERROR);
                return;
            });
            if (value == 0)
                thread_id = resolveCurrentThreadId();
            else if (value > 0)
                thread_id = static_cast<thread_id_t>(value);
        }

        parsed.push_back({.kind = kind, .thread_id = thread_id});
    }

    std::vector<ResumeAction> actions;
    for (const auto id : getTarget().getThreadIds()) {
        for (const auto& action : parsed) {
            if (!action.thread_id || action.thread_id.value() == id) {
                actions.push_back({.id = id, .kind = action.kind});
                break;
            }
        }
    }

    resumeTarget(actions);
}

void GdbServer::handleQuery(std::string_view command) {
    if (command == "StartNoAckMode") {
        do_ack = false;
        sendPacket(GDB_OK);
    } else if (command.starts_with("Supported")) {
        sendPacket(fmt::format("PacketSize={:x};qXfer:features:read+;"
                               "qXfer:libraries:read+;vContSupported+;"
                               "QStartNoAckMode+",
                               MAX_PACKET_SIZE));
    } else if (command == "C") {
        const auto id = resolveCurrentThreadId();
        if (id == invalid<thread_id_t>())
            sendPacket(GDB_EMPTY);
        else
            sendPacket(fmt::format("QC{:x}", id));
    } else if (command.starts_with("Attached")) {
        sendPacket("1");
    } else if (command == "Offsets") {
        sendPacket(GDB_EMPTY);
    } else if (command.starts_with("WatchpointSupportInfo")) {
        sendPacket(
            fmt::format("num:{};", getTarget().getWatchpointSlotCount()));
    } else if (command.starts_with("ThreadStopInfo")) {
        handleThreadStopInfo(command.substr(14));
    } else if (command == "HostInfo") {
        sendPacket(fmt::format("triple:{};ptrsize:8;endian:little;"
                               "watchpoint_exceptions_received:before;",
                               hexEncode(HOST_TRIPLE)));
    } else if (command == "fThreadInfo") {
        const auto ids = getTarget().getThreadIds();
        std::vector<std::string> thread_ids;
        thread_ids.reserve(ids.size());
        for (const auto id : ids)
            thread_ids.push_back(fmt::format("{:x}", id));
        sendPacket(fmt::format("m{}", fmt::join(thread_ids, ",")));
    } else if (command == "sThreadInfo") {
        sendPacket("l");
    } else if (command.starts_with("Xfer:features:read:target.xml:")) {
        const auto& target_xml = getTargetXmlAarch64();
        if (const auto page = pageFromBuffer(target_xml, command.substr(30)))
            sendPacket(page.value());
        else
            sendPacket(GDB_ERROR);
    } else if (command.starts_with("Xfer:libraries:read::")) {
        std::string output =
            R"(<?xml version="1.0"?><library-list version="1.0">)";
        for (const auto& symbol : debugger.getModuleTable().getSymbols()) {
            output += fmt::format(
                R"(<library name="{}"><segment address="{:#x}"/></library>)",
                symbol.name, symbol.guest_mem_range.getBegin());
        }
        output += "</library-list>";
        if (const auto page = pageFromBuffer(output, command.substr(21)))
            sendPacket(page.value());
        else
            sendPacket(GDB_ERROR);
    } else if (command.starts_with("Rcmd,")) {
        handleRcmd(command.substr(5));
    } else {
        LOG_WARN(Debugger, "Unhandled GDB query: {}", loggedPacket(command));
        sendPacket(GDB_EMPTY);
    }
}

void GdbServer::handleSetActiveThread(std::string_view command) {
    if (command.empty()) {
        sendPacket(GDB_ERROR);
        return;
    }

    ZTD_ASSIGN_OR(const auto thread_id, fromChars<i64>(command.substr(1), 16), {
        sendPacket(GDB_ERROR);
        return;
    });

    // Any thread
    if (thread_id <= 0) {
        crnt_thread_id = resolveCurrentThreadId();
        sendPacket(GDB_OK);
        return;
    }

    const auto id = static_cast<thread_id_t>(thread_id);
    if (!getTarget().hasThread(id)) {
        sendPacket(GDB_ERROR);
        return;
    }

    crnt_thread_id = id;
    sendPacket(GDB_OK);
}

void GdbServer::handleThreadStatus() {
    waitForAllStopped(INITIAL_STOP_TIMEOUT_NS);
    const auto id = resolveCurrentThreadId();
    if (id == invalid<thread_id_t>()) {
        sendPacket(GDB_ERROR);
        return;
    }

    sendPacket(formatStopReply(id, signalForThread(id, Signal::SigTrap)));
}

void GdbServer::handleThreadStopInfo(std::string_view command) {
    ZTD_ASSIGN_OR(const auto id, fromChars<thread_id_t>(command, 16), {
        sendPacket(GDB_ERROR);
        return;
    });

    if (!getTarget().hasThread(id)) {
        sendPacket(GDB_ERROR);
        return;
    }

    sendPacket(formatStopReply(id, signalForThread(id, Signal::SigZero)));
}

void GdbServer::handleThreadAlive(std::string_view command) {
    ZTD_ASSIGN_OR(const auto id, fromChars<thread_id_t>(command, 16), {
        sendPacket(GDB_ERROR);
        return;
    });

    sendPacket(getTarget().hasThread(id) ? GDB_OK : GDB_ERROR);
}

void GdbServer::handleDetach() {
    sendPacket(GDB_OK);
    session_end = SessionEnd::Detached;
}

void GdbServer::handleKill() {
    sendPacket("X09");
    session_end = SessionEnd::Killed;
}

void GdbServer::handleRegRead(std::string_view command) {
    ZTD_ASSIGN_OR(const auto id, fromChars<u32>(command, 16), {
        sendPacket(GDB_ERROR);
        return;
    });

    std::optional<std::string> hex;
    {
        auto stopped =
            getTarget().tryAcquireStoppedThread(resolveCurrentThreadId());
        if (stopped)
            hex = formatReg(stopped->getState(), id);
    }

    if (hex)
        sendPacket(hex.value());
    else
        sendPacket(GDB_ERROR);
}

void GdbServer::handleRegReadAll() {
    std::optional<std::string> output;
    {
        auto stopped =
            getTarget().tryAcquireStoppedThread(resolveCurrentThreadId());
        if (stopped) {
            output.emplace();
            output->reserve(REG_FILE_HEX_SIZE);
            for (u32 id = 0; id <= FPCR_REGISTER; id++) {
                const auto hex = formatReg(stopped->getState(), id);
                if (!hex) {
                    output.reset();
                    break;
                }

                *output += hex.value();
            }
        }
    }

    if (output)
        sendPacket(output.value());
    else
        sendPacket(GDB_ERROR);
}

void GdbServer::handleRegWrite(std::string_view command) {
    const auto eq_pos = command.find('=');
    if (eq_pos == std::string_view::npos) {
        sendPacket(GDB_ERROR);
        return;
    }

    const auto id_str = command.substr(0, eq_pos);
    const auto value_str = command.substr(eq_pos + 1);
    ZTD_ASSIGN_OR(const auto id, fromChars<u32>(id_str, 16), {
        sendPacket(GDB_ERROR);
        return;
    });

    bool ok = false;
    {
        auto stopped =
            getTarget().tryAcquireStoppedThread(resolveCurrentThreadId());
        if (stopped)
            ok = tryParseReg(stopped->getState(), id, value_str);
    }

    sendPacket(ok ? GDB_OK : GDB_ERROR);
}

void GdbServer::handleRegWriteAll(std::string_view command) {
    if (command.size() != REG_FILE_HEX_SIZE) {
        sendPacket(GDB_ERROR);
        return;
    }

    bool ok = false;
    {
        auto stopped =
            getTarget().tryAcquireStoppedThread(resolveCurrentThreadId());
        if (stopped) {
            auto state = stopped->getState();
            ok = true;
            usize offset = 0;
            for (u32 id = 0; id <= FPCR_REGISTER; id++) {
                const auto width = regHexWidth(id);
                if (!tryParseReg(state, id, command.substr(offset, width))) {
                    ok = false;
                    break;
                }

                offset += width;
            }

            if (ok)
                stopped->getState() = state;
        }
    }

    sendPacket(ok ? GDB_OK : GDB_ERROR);
}

void GdbServer::handleMemRead(std::string_view command) {
    const auto request = parseMemoryRequest(command);
    if (!request) {
        sendPacket(GDB_ERROR);
        return;
    }

    std::vector<u8> data(request->size);
    if (!getTarget().tryReadMemory(request->addr, std::span(data))) {
        sendPacket(GDB_ERROR);
        return;
    }

    std::string output;
    output.reserve(request->size * 2);
    for (const auto value : data)
        output += fmt::format("{:02x}", value);

    sendPacket(output);
}

void GdbServer::handleMemWrite(std::string_view command) {
    const auto request = parseMemoryRequest(command);
    if (!request || request->payload.size() < request->size * 2) {
        sendPacket(GDB_ERROR);
        return;
    }

    std::vector<u8> data(request->size);
    for (u64 i = 0; i < request->size; i++) {
        ZTD_ASSIGN_OR(data[i],
                      fromChars<u8>(request->payload.substr(i * 2, 2), 16), {
                          sendPacket(GDB_ERROR);
                          return;
                      });
    }

    sendPacket(
        getTarget().tryWriteMemory(request->addr, std::span<const u8>(data))
            ? GDB_OK
            : GDB_ERROR);
}

void GdbServer::handleMemWriteBinary(std::string_view command) {
    const auto request = parseMemoryRequest(command);
    if (!request) {
        sendPacket(GDB_ERROR);
        return;
    }

    if (request->size == 0) {
        sendPacket(GDB_OK);
        return;
    }

    const auto data = unescapeBinary(request->payload, request->size);
    if (!data) {
        sendPacket(GDB_ERROR);
        return;
    }

    sendPacket(getTarget().tryWriteMemory(request->addr,
                                          std::span<const u8>(data.value()))
                   ? GDB_OK
                   : GDB_ERROR);
}

void GdbServer::handleInsertBreakpoint(std::string_view command) {
    const auto request = parseBreakpointRequest(command);
    if (!request) {
        sendPacket(GDB_ERROR);
        return;
    }

    switch (request->type) {
    case BreakpointType::Software:
        sendPacket(request->size == sizeof(u32) &&
                           getTarget().insertBreakpoint(request->addr)
                       ? GDB_OK
                       : GDB_ERROR);
        break;
    case BreakpointType::Hardware:
        if (!getTarget().hasNativeBreakpoints())
            sendPacket(GDB_EMPTY);
        else
            sendPacket(getTarget().insertBreakpoint(request->addr) ? GDB_OK
                                                                   : GDB_ERROR);
        break;
    default: {
        const auto watchpoint = toWatchpoint(request.value());
        if (!watchpoint || getTarget().getWatchpointSlotCount() == 0)
            sendPacket(GDB_EMPTY);
        else
            sendPacket(getTarget().insertWatchpoint(watchpoint.value())
                           ? GDB_OK
                           : GDB_ERROR);
        break;
    }
    }
}

void GdbServer::handleRemoveBreakpoint(std::string_view command) {
    const auto request = parseBreakpointRequest(command);
    if (!request) {
        sendPacket(GDB_ERROR);
        return;
    }

    switch (request->type) {
    case BreakpointType::Software:
        sendPacket(request->size == sizeof(u32) &&
                           getTarget().removeBreakpoint(request->addr)
                       ? GDB_OK
                       : GDB_ERROR);
        break;
    case BreakpointType::Hardware:
        if (!getTarget().hasNativeBreakpoints())
            sendPacket(GDB_EMPTY);
        else
            sendPacket(getTarget().removeBreakpoint(request->addr) ? GDB_OK
                                                                   : GDB_ERROR);
        break;
    default: {
        const auto watchpoint = toWatchpoint(request.value());
        if (!watchpoint || getTarget().getWatchpointSlotCount() == 0)
            sendPacket(GDB_EMPTY);
        else
            sendPacket(getTarget().removeWatchpoint(watchpoint.value())
                           ? GDB_OK
                           : GDB_ERROR);
        break;
    }
    }
}

void GdbServer::handleRcmd(std::string_view cmd) {
    if (cmd.size() % 2 != 0) {
        sendPacket(GDB_ERROR);
        return;
    }

    std::string command;
    command.reserve(cmd.size() / 2);
    for (usize i = 0; i < cmd.size(); i += 2) {
        ZTD_ASSIGN_OR(const auto c, fromChars<u8>(cmd.substr(i, 2), 16), {
            sendPacket(GDB_ERROR);
            return;
        });
        command += static_cast<char>(c);
    }

    if (command == "getExecutables") {
        handleGetExecutables();
        return;
    }

    std::string_view args = command;
    const auto name = nextWord(args);
    if (name == "help") {
        handleMonitorHelp();
    } else if (name == "disassemble") {
        handleMonitorDisassemble(args, false);
    } else if (name == "disassemble-function") {
        handleMonitorDisassemble(args, true);
    } else {
        LOG_WARN(Debugger, "Unhandled GDB RCMD: {}", command);
        sendPacket(GDB_EMPTY);
    }
}

void GdbServer::handleMonitorHelp() {
    sendConsoleOutput("disassemble [addr|pc] [count]\n"
                      "disassemble-function [addr|pc]\n"
                      "getExecutables\n");
    sendPacket(GDB_OK);
}

void GdbServer::handleMonitorDisassemble(std::string_view args, bool function) {
    const auto addr = resolveMonitorAddress(nextWord(args));
    if (!addr) {
        sendMonitorError(addr.error());
        return;
    }

    const auto pc = tryGetCurrentPc();
    std::expected<Listing, DisassemblyError> listing;
    if (function) {
        listing = debugger.disassembleFunction(*addr, pc);
    } else {
        u64 count = DEFAULT_LISTING_INSTRUCTIONS;
        if (const auto count_arg = nextWord(args); !count_arg.empty()) {
            const auto parsed = parseMonitorNumber(count_arg, false);
            if (!parsed || *parsed == 0) {
                sendMonitorError("invalid count");
                return;
            }

            count = *parsed;
        }

        listing =
            debugger.disassemble({.addr = *addr,
                                  .count = static_cast<u32>(std::min<u64>(
                                      count, std::numeric_limits<u32>::max())),
                                  .pc = pc});
    }

    if (!listing) {
        sendMonitorError(fmt::format("{}", listing.error()));
        return;
    }

    auto text = formatListing(*listing);
    if (listing->truncated)
        text += "(truncated)\n";
    sendConsoleOutput(text);
    sendPacket(GDB_OK);
}

void GdbServer::sendConsoleOutput(std::string_view text) {
    for (usize offset = 0; offset < text.size();
         offset += MAX_CONSOLE_CHUNK_SIZE) {
        const auto chunk = text.substr(offset, MAX_CONSOLE_CHUNK_SIZE);
        sendPacket(fmt::format("O{}", hexEncode(chunk)));
    }
}

void GdbServer::sendMonitorError(std::string_view message) {
    sendConsoleOutput(fmt::format("error: {}\n", message));
    sendPacket(GDB_ERROR);
}

std::expected<vaddr_t, std::string_view>
GdbServer::resolveMonitorAddress(std::string_view arg) {
    if (arg.empty() || arg == "pc") {
        if (const auto pc = tryGetCurrentPc())
            return *pc;

        return std::unexpected("no stopped thread for pc");
    }

    if (const auto addr = parseMonitorNumber(arg, true))
        return *addr;

    return std::unexpected("invalid address");
}

std::optional<vaddr_t> GdbServer::tryGetCurrentPc() {
    const auto id = resolveCurrentThreadId();
    if (id == invalid<thread_id_t>())
        return std::nullopt;

    const auto stopped = getTarget().tryAcquireStoppedThread(id);
    if (!stopped)
        return std::nullopt;

    return stopped->getState().pc;
}

void GdbServer::handleGetExecutables() {
    std::string dir_path = fmt::format("/tmp/hydra/dump/executables/{:016x}",
                                       getTarget().getTitleId());
    std::filesystem::create_directories(dir_path);

    std::string output;
    for (const auto& module_ : debugger.getModuleTable().getSymbols()) {
        const auto file_it = debugger.getExecutables().find(module_.name);
        if (file_it == debugger.getExecutables().end()) {
            LOG_WARN(Debugger, "No executable registered for module {}",
                     module_.name);
            continue;
        }

        std::string path = fmt::format("{}/{}.elf", dir_path, module_.name);

        // Output
        if (!output.empty())
            output += ';';
        output += fmt::format("\"{}\":{:#x}", path,
                              module_.guest_mem_range.getBegin());

        // Save the executable
        if (std::filesystem::exists(path)) {
            LOG_INFO(Debugger, "Symbol file {} already exists, skipping",
                     module_.name);
            continue;
        }

        // Load the executable
        auto stream =
            file_it->second->open(horizon::filesystem::FileOpenFlags::Read);

        std::vector<u8> data(stream->getSize());
        stream->readToSpan(std::span(data));

        delete stream;

        // Convert to ELF
        NsoFile nso;
        nso.Load(data);
        nso.WriteElf(path);
    }

    sendPacket(output);
}

void GdbServer::resumeTarget() {
    beginResume();
    getTarget().resumeAll();
}

void GdbServer::resumeTarget(std::span<const ResumeAction> actions) {
    if (actions.empty() || !getTarget().resume(actions)) {
        replyCurrentStop();
        return;
    }

    beginResume();
    for (const auto& action : actions) {
        if (action.kind == ResumeKind::Step) {
            crnt_thread_id = action.id;
            break;
        }
    }
}

void GdbServer::stepTargetThread(thread_id_t id) {
    const ResumeAction action{.id = id, .kind = ResumeKind::Step};
    resumeTarget(std::span<const ResumeAction>(&action, 1));
}

void GdbServer::replyCurrentStop() {
    if (!getTarget().isProcessRunning()) {
        sendPacket("W00");
        session_end = SessionEnd::ProcessExited;
        return;
    }

    const auto id = resolveCurrentThreadId();
    if (id == invalid<thread_id_t>()) {
        sendPacket("T05");
        return;
    }

    sendPacket(formatStopReply(id, signalForThread(id, Signal::SigTrap)));
}

void GdbServer::beginResume() {
    awaiting_stop_reply = true;
    stop_deadline.reset();
    interrupted_thread.reset();
}

thread_id_t GdbServer::resolveCurrentThreadId() {
    auto& target = getTarget();
    if (crnt_thread_id != invalid<thread_id_t>() &&
        target.hasThread(crnt_thread_id))
        return crnt_thread_id;

    crnt_thread_id = invalid<thread_id_t>();
    const auto ids = target.getThreadIds();
    for (const auto id : ids) {
        if (target.getStopInfo(id)) {
            crnt_thread_id = id;
            return id;
        }
    }

    if (!ids.empty())
        crnt_thread_id = ids.front();

    return crnt_thread_id;
}

void GdbServer::setNonBlocking(i32 socket) {
    i32 flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

std::optional<std::string>
GdbServer::formatReg(const hw::tegra_x1::cpu::ThreadState& state, u32 id) {
    switch (id) {
    case 0 ... 28:
        return numberToHex(state.r[id]);
    case 29:
        return numberToHex(state.fp);
    case 30:
        return numberToHex(state.lr);
    case 31:
        return numberToHex(state.sp);
    case 32:
        return numberToHex(state.pc);
    case 33:
        return numberToHex(state.pstate);
    case 34 ... 65:
        return numberToHex(state.v[id - Q0_REGISTER]);
    case 66:
        return numberToHex(state.fpsr);
    case 67:
        return numberToHex(state.fpcr);
    default:
        return std::nullopt;
    }
}

bool GdbServer::tryParseReg(hw::tegra_x1::cpu::ThreadState& state, u32 id,
                            std::string_view hex) {
    switch (id) {
    case 0 ... 28: {
        ZTD_ASSIGN_OR_RETURN_VALUE(state.r[id], hexToNumber<u64>(hex), false);
        return true;
    }
    case 29: {
        ZTD_ASSIGN_OR_RETURN_VALUE(state.fp, hexToNumber<u64>(hex), false);
        return true;
    }
    case 30: {
        ZTD_ASSIGN_OR_RETURN_VALUE(state.lr, hexToNumber<u64>(hex), false);
        return true;
    }
    case 31: {
        ZTD_ASSIGN_OR_RETURN_VALUE(state.sp, hexToNumber<u64>(hex), false);
        return true;
    }
    case 32: {
        ZTD_ASSIGN_OR_RETURN_VALUE(state.pc, hexToNumber<u64>(hex), false);
        return true;
    }
    case 33: {
        ZTD_ASSIGN_OR_RETURN_VALUE(state.pstate, hexToNumber<u32>(hex), false);
        return true;
    }
    case 34 ... 65: {
        ZTD_ASSIGN_OR_RETURN_VALUE(state.v[id - Q0_REGISTER],
                                   hexToNumber<u128>(hex), false);
        return true;
    }
    case 66: {
        ZTD_ASSIGN_OR_RETURN_VALUE(state.fpsr, hexToNumber<u32>(hex), false);
        return true;
    }
    case 67: {
        ZTD_ASSIGN_OR_RETURN_VALUE(state.fpcr, hexToNumber<u32>(hex), false);
        return true;
    }
    default:
        return false;
    }
}

std::string GdbServer::formatStopReply(thread_id_t id, Signal signal) {
    std::string reply = fmt::format("T{:02x}", static_cast<u8>(signal));
    if (const auto stop = getTarget().getStopInfo(id);
        stop && stop->watchpoint) {
        reply += fmt::format("{}:{:x};", watchpointKey(stop->watchpoint->type),
                             stop->watchpoint->addr);
    }

    if (const auto stopped = getTarget().tryAcquireStoppedThread(id)) {
        const auto& state = stopped->getState();
        reply += fmt::format("{:02x}:{};{:02x}:{};{:02x}:{};", PC_REGISTER,
                             numberToHex(state.pc), SP_REGISTER,
                             numberToHex(state.sp), LR_REGISTER,
                             numberToHex(state.lr));
    }

    reply += fmt::format("thread:{:x};", id);
    return reply;
}

Signal GdbServer::signalForThread(thread_id_t id, Signal fallback) const {
    if (const auto stop = getTarget().getStopInfo(id))
        return signalFromStop(stop.value());

    if (interrupted_thread == id)
        return Signal::SigInt;

    return fallback;
}

Signal GdbServer::signalFromStop(const StopInfo& stop) {
    if (stop.reason == hw::tegra_x1::cpu::HaltReason::Fault)
        return Signal::SigSegv;

    return Signal::SigTrap;
}

Target& GdbServer::getTarget() const { return debugger.getTarget(); }

std::optional<std::string> GdbServer::pageFromBuffer(std::string_view buffer,
                                                     std::string_view page) {
    const auto comma_pos = page.find(',');
    if (comma_pos == std::string_view::npos)
        return std::nullopt;

    ZTD_ASSIGN_OR_RETURN_VALUE(const auto offset,
                               fromChars<u64>(page.substr(0, comma_pos), 16),
                               std::nullopt);
    ZTD_ASSIGN_OR_RETURN_VALUE(const auto size,
                               fromChars<u64>(page.substr(comma_pos + 1), 16),
                               std::nullopt);

    if (offset >= buffer.size())
        return "l";
    if (offset + size < buffer.size())
        return fmt::format("m{}", buffer.substr(offset, size));
    return fmt::format("l{}", buffer.substr(offset));
}

} // namespace hydra::debugger
