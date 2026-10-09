#include "core/debugger/gdb_server.hpp"

#include <netinet/in.h>
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

[[maybe_unused]] constexpr char GDB_INT3 = 0x03;
[[maybe_unused]] constexpr int GDB_SIGTRAP = 5;

constexpr char GDB_OK[] = "OK";
constexpr char GDB_ERROR[] = "E01";
constexpr char GDB_EMPTY[] = "";

[[maybe_unused]] constexpr u32 FP_REGISTER = 29;
constexpr u32 LR_REGISTER = 30;
constexpr u32 SP_REGISTER = 31;
constexpr u32 PC_REGISTER = 32;
[[maybe_unused]] constexpr u32 PSTATE_REGISTER = 33;
[[maybe_unused]] constexpr u32 Q0_REGISTER = 34;
[[maybe_unused]] constexpr u32 FPSR_REGISTER = 66;
[[maybe_unused]] constexpr u32 FPCR_REGISTER = 67;

enum class BreakpointType {
    Software = 0,
    Hardware = 1,
    WriteWatch = 2,
    ReadWatch = 3,
    AccessWatch = 4,
};

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
    T value = 0;
    for (usize i = 0; i < sizeof(T); ++i) {
        ZTD_ASSIGN_OR_RETURN_ERROR(const auto crnt,
                                   fromChars<T>(hex.substr(i * 2, 2), 16));
        value |= crnt << (i * 8);
    }

    return value;
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
        return;
    }

    // Start listening
    if (listen(server_socket, 1) == -1) {
        LOG_ERROR(Debugger, "Failed to listen on GDB socket");
        close(server_socket);
        return;
    }

    // Set the socket to non-blocking mode
    setNonBlocking(server_socket);

    // Create server thread
    server_thread = std::thread(&GdbServer::serverLoop, this);

    LOG_INFO(Debugger, "GDB server started on port {}", port);

    // Thread ID
    const auto thread_ids = getTarget().getThreadIds();
    ASSERT(!thread_ids.empty(), Debugger, "No active thread");
    crnt_thread_id = thread_ids.front();
}

GdbServer::~GdbServer() {
    running = false;
    server_thread.join();
    close(server_socket);
}

void GdbServer::notifyHalted(thread_id_t id,
                            hw::tegra_x1::cpu::HaltReason reason) {
    std::scoped_lock lock(mutex);
    if (client_socket == -1)
        return;

    crnt_thread_id = id;

    if (const auto status = getThreadStatus(id, signalFromHaltReason(reason)))
        sendPacket(status.value());
}

void GdbServer::closeClientSocket() {
    ASSERT(client_socket != -1, Debugger, "Client socket is not open");
    close(client_socket);
    client_socket = -1;
    LOG_INFO(Debugger, "GDB client disconnected");
}

void GdbServer::serverLoop() {
    GET_CURRENT_PROCESS_DEBUGGER().registerThisThread("GDB server");
    while (running) {
        poll();
        // TODO: is the delay necessary?
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    GET_CURRENT_PROCESS_DEBUGGER().unregisterThisThread();
}

void GdbServer::poll() {
    if (client_socket == -1) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        i32 new_client =
            accept(server_socket, reinterpret_cast<sockaddr*>(&client_addr),
                   &addr_len);

        if (new_client != -1) {
            client_socket = new_client;
            setNonBlocking(client_socket);
            getTarget().pauseAll();
        }
    } else {
        char buffer[1024];
        const auto bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_read > 0) {
            receive_buffer +=
                std::string_view(buffer, static_cast<usize>(bytes_read));
            processPackets();
        } else if (bytes_read == 0) {
            closeClientSocket();
            // TODO: also resume all threads?
        }
    }
}

void GdbServer::processPackets() {
    usize str_start = 0;
    while (true) {
        const auto start = receive_buffer.find(GDB_START, str_start);
        if (start == std::string::npos)
            break;

        const auto end = receive_buffer.find(GDB_END, start);
        if (end == std::string::npos)
            break;

        std::string command = receive_buffer.substr(start + 1, end - start - 1);
        if (end + 2 < receive_buffer.size()) {
            sendStatus(GDB_ACK);
            handleCommand(command);
        } else {
            sendStatus(GDB_NACK);
            break;
        }

        str_start = end + 3;
    }

    if (str_start > 0)
        receive_buffer = receive_buffer.substr(str_start);
}

void GdbServer::handleCommand(std::string_view command) {
    // TODO: only log in debug
    LOG_INFO(Debugger, "COMMAND: {}", command);

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
        getTarget().stop();
        closeClientSocket();
        break;
    case 'H':
        handleSetActiveThread(body);
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
    case 'm':
        handleMemRead(body);
        break;
    case 'M':
        handleMemWrite(body);
        break;
    case 'c':
        getTarget().resumeAll();
        break;
    case 'Z':
        handleInsertBreakpoint(body);
        break;
    case 'z':
        handleRemoveBreakpoint(body);
        break;
    default:
        LOG_WARN(Debugger, "Unhandled GDB command: {}", command);
        sendPacket(GDB_EMPTY);
        break;
    }
}

void GdbServer::handleVCont(std::string_view command) {
    if (command == "?") {
        sendPacket("vCont;c;C;s;S");
        return;
    }

    auto step_thread_id = invalid<thread_id_t>();
    bool lock_execution = true; // TODO: what is this?

    usize pos = 1;
    do {
        const auto next_pos = command.find(';', pos);
        const auto entry = command.substr(pos, (next_pos != std::string::npos
                                                    ? next_pos - pos
                                                    : std::string::npos));
        pos = next_pos;

        switch (entry[0]) {
        case 'c':
        case 'C':
            lock_execution = false;
            break;
        case 's':
        case 'S': {
            const auto thread_id_str = entry.substr(entry.find(':') + 1);
            ZTD_ASSIGN_OR(
                const auto thread_id, fromChars<i64>(thread_id_str, 16),
                LOG_FATAL(Debugger, "Invalid thread ID {}", thread_id_str));
            if (thread_id > 0)
                step_thread_id = static_cast<thread_id_t>(thread_id);
            break;
        }
        default:
            LOG_WARN(Debugger, "Unhandled GDB vCont entry: {}", entry[0]);
            sendPacket(GDB_EMPTY);
            break;
        }
    } while (pos != std::string::npos);

    // TODO: how does this work?
    if (step_thread_id != invalid<thread_id_t>()) {
        // TODO: implement
        ASSERT_DEBUG(lock_execution, Debugger,
                     "Non-locked execution not implemented");
        crnt_thread_id = step_thread_id;

        if (!getTarget().stepThread(step_thread_id))
            sendPacket(GDB_ERROR);
    } else {
        getTarget().resumeAll();
    }
}

void GdbServer::handleQuery(std::string_view command) {
    if (command == "StartNoAckMode") {
        do_ack = false;
        sendPacket(GDB_OK);
    } else if (command.starts_with("Supported")) {
        // TODO: why packet size 4000?
        sendPacket("PacketSize=4000;qXfer:features:read+;qXfer:threads:read+;"
                   "qXfer:libraries:read+;"
                   "vContSupported+;QStartNoAckMode+");
    } else if (command.starts_with("fThreadInfo")) { // TODO: ==?
        const auto ids = getTarget().getThreadIds();
        std::vector<std::string> thread_ids;
        thread_ids.reserve(ids.size());
        for (const auto id : ids)
            thread_ids.push_back(fmt::format("{:x}", id));
        sendPacket(fmt::format("m{}", fmt::join(thread_ids, ",")));
    } else if (command.starts_with("sThreadInfo")) { // TODO: ==?
        // TODO: why?
        sendPacket("l");
    } else if (command.starts_with("Xfer:features:read:target.xml:")) {
        const auto& target_xml = getTargetXmlAarch64();
        sendPacket(pageFromBuffer(target_xml, command.substr(30)));
    } else if (command.starts_with("Xfer:libraries:read::")) {
        std::string output = R"(<?xml version="1.0"?><library-list>)";
        for (const auto& symbol : debugger.getModuleTable().getSymbols()) {
            // TODO: what's the purpose of this?
            // TODO: number_to_hex?
            output += fmt::format(
                R"(<library name="{}"><segment address="{:#x}"/></library>)",
                symbol.name, symbol.guest_mem_range.getBegin());
        }
        output += "</library-list>";
        sendPacket(pageFromBuffer(output, command.substr(21)));
    } else if (command.starts_with("Rcmd")) {
        handleRcmd(command.substr(5));
    } else {
        LOG_WARN(Debugger, "Unhandled GDB query: {}", command);
        sendPacket(GDB_EMPTY);
    }
}

void GdbServer::handleSetActiveThread(std::string_view command) {
    const auto thread_id_str = command.substr(1);
    ZTD_ASSIGN_OR(const auto thread_id, fromChars<i64>(thread_id_str, 16),
                  LOG_FATAL(Debugger, "Invalid thread ID {}", thread_id_str));

    // Any thread
    if (thread_id <= 0) {
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
    const auto reason = getTarget().getHaltReason(crnt_thread_id);
    const auto signal =
        reason ? signalFromHaltReason(reason.value()) : Signal::SigTrap;

    if (const auto status = getThreadStatus(crnt_thread_id, signal))
        sendPacket(status.value());
    else
        sendPacket(GDB_ERROR);
}

void GdbServer::handleRegRead(std::string_view command) {
    ZTD_ASSIGN_OR(const auto id, fromChars<u32>(command, 16),
                  LOG_FATAL(Debugger, "Invalid reg ID {}", command));

    std::optional<std::string> hex;
    {
        auto stopped = getTarget().tryAcquireStoppedThread(crnt_thread_id);
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
        auto stopped = getTarget().tryAcquireStoppedThread(crnt_thread_id);
        if (stopped) {
            output.emplace();
            output->reserve(static_cast<usize>(FPCR_REGISTER + 1) * 16);
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
    ZTD_ASSIGN_OR(const auto id, fromChars<u32>(id_str, 16),
                  LOG_FATAL(Debugger, "Invalid reg ID {}", id_str));

    bool ok = false;
    {
        auto stopped = getTarget().tryAcquireStoppedThread(crnt_thread_id);
        if (stopped)
            ok = tryParseReg(stopped->getState(), id, value_str);
    }

    sendPacket(ok ? GDB_OK : GDB_ERROR);
}

void GdbServer::handleMemRead(std::string_view command) {
    const auto comma_pos = command.find(',');
    const auto addr_str = command.substr(0, comma_pos);
    const auto size_str = command.substr(comma_pos + 1);
    ZTD_ASSIGN_OR(const auto addr, fromChars<u64>(addr_str, 16),
                  LOG_FATAL(Debugger, "Invalid address {}", addr_str));
    ZTD_ASSIGN_OR(const auto size, fromChars<u64>(size_str, 16),
                  LOG_FATAL(Debugger, "Invalid size {}", size_str));

    std::vector<u8> data(size);
    if (!getTarget().tryReadMemory(addr, std::span(data))) {
        sendPacket(GDB_ERROR);
        return;
    }

    std::string output;
    output.reserve(size * 2);
    for (const auto value : data)
        output += fmt::format("{:02x}", value);

    sendPacket(output);
}

void GdbServer::handleMemWrite(std::string_view command) {
    const auto comma_pos = command.find(',');
    const auto colon_pos = command.find(':', comma_pos);
    if (comma_pos == std::string_view::npos ||
        colon_pos == std::string_view::npos) {
        sendPacket(GDB_ERROR);
        return;
    }

    const auto addr_str = command.substr(0, comma_pos);
    const auto size_str =
        command.substr(comma_pos + 1, colon_pos - comma_pos - 1);
    const auto data_str = command.substr(colon_pos + 1);
    ZTD_ASSIGN_OR(const auto addr, fromChars<u64>(addr_str, 16),
                  LOG_FATAL(Debugger, "Invalid address {}", addr_str));
    ZTD_ASSIGN_OR(const auto size, fromChars<u64>(size_str, 16),
                  LOG_FATAL(Debugger, "Invalid size {}", size_str));

    if (data_str.size() < size * 2) {
        sendPacket(GDB_ERROR);
        return;
    }

    std::vector<u8> data(size);
    for (u64 i = 0; i < size; i++) {
        ZTD_ASSIGN_OR(data[i], fromChars<u8>(data_str.substr(i * 2, 2), 16), {
            sendPacket(GDB_ERROR);
            return;
        });
    }

    sendPacket(getTarget().tryWriteMemory(addr, std::span<const u8>(data))
                   ? GDB_OK
                   : GDB_ERROR);
}

void GdbServer::handleInsertBreakpoint(std::string_view command) {
    const auto addr_pos = command.find(',') + 1;
    const auto size_pos = command.find(',', addr_pos) + 1;

    const auto type_str = command.substr(0, addr_pos - 1);
    const auto addr_str = command.substr(addr_pos, size_pos - addr_pos - 1);
    const auto size_str = command.substr(size_pos);
    ZTD_ASSIGN_OR(const auto type_int, fromChars<u8>(type_str, 16),
                  LOG_FATAL(Debugger, "Invalid type {}", type_str));
    const auto type = static_cast<BreakpointType>(type_int);
    ZTD_ASSIGN_OR(const auto addr, fromChars<u64>(addr_str, 16),
                  LOG_FATAL(Debugger, "Invalid address {}", addr_str));
    ZTD_ASSIGN_OR(const auto size, fromChars<u64>(size_str, 16),
                  LOG_FATAL(Debugger, "Invalid size {}", size_str));

    // NOLINTNEXTLINE(readability-trivial-switch)
    switch (type) {
    case BreakpointType::Software:
        ASSERT_DEBUG(size == 4, Debugger,
                     "Invalid software breakpoint size 0x{:x}", size);

        sendPacket(getTarget().insertBreakpoint(addr) ? GDB_OK : GDB_ERROR);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Debugger, "Breakpoint type {}", type);
        sendPacket(GDB_ERROR);
        break;
    }
}

void GdbServer::handleRemoveBreakpoint(std::string_view command) {
    const auto addr_pos = command.find(',') + 1;
    const auto size_pos = command.find(',', addr_pos) + 1;

    const auto type_str = command.substr(0, addr_pos - 1);
    const auto addr_str = command.substr(addr_pos, size_pos - addr_pos - 1);
    const auto size_str = command.substr(size_pos);
    ZTD_ASSIGN_OR(const auto type_int, fromChars<u8>(type_str, 16),
                  LOG_FATAL(Debugger, "Invalid type {}", type_str));
    const auto type = static_cast<BreakpointType>(type_int);
    ZTD_ASSIGN_OR(const auto addr, fromChars<u64>(addr_str, 16),
                  LOG_FATAL(Debugger, "Invalid address {}", addr_str));
    ZTD_ASSIGN_OR(const auto size, fromChars<u64>(size_str, 16),
                  LOG_FATAL(Debugger, "Invalid size {}", size_str));

    // NOLINTNEXTLINE(readability-trivial-switch)
    switch (type) {
    case BreakpointType::Software:
        ASSERT_DEBUG(size == 4, Debugger,
                     "Invalid software breakpoint size 0x{:x}", size);

        sendPacket(getTarget().removeBreakpoint(addr) ? GDB_OK : GDB_ERROR);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Debugger, "Breakpoint type {}", type);
        sendPacket(GDB_ERROR);
        break;
    }
}

void GdbServer::handleRcmd(std::string_view cmd) {
    std::string command;
    command.reserve(cmd.size() / 2);
    for (u32 i = 0; i < cmd.size(); i += 2) {
        const auto str = cmd.substr(i, 2);
        char c;
        auto [ptr, ec] =
            std::from_chars(str.data(), str.data() + str.size(), c, 16);
        ASSERT_DEBUG(ec == std::errc(), Debugger, "Failed to parse command");
        command += c;
    }

    if (command == "getExecutables") {
        handleGetExecutables();
    } else {
        LOG_WARN(Debugger, "Unhandled GDB RCMD: {}", command);
        sendPacket(GDB_EMPTY);
    }
}

void GdbServer::handleGetExecutables() {
    std::string dir_path = fmt::format("/tmp/hydra/dump/executables/{:016x}",
                                       getTarget().getTitleId());
    std::filesystem::create_directories(dir_path);

    std::string output;
    for (u32 i = 0; i < debugger.getModuleTable().getSymbols().size(); i++) {
        const auto& module_ = debugger.getModuleTable().getSymbols()[i];
        std::string path = fmt::format("{}/{}.elf", dir_path, module_.name);

        // Output
        output += fmt::format("\"{}\":{:#x}", path,
                              module_.guest_mem_range.getBegin());
        if (i < debugger.getModuleTable().getSymbols().size() - 1)
            output += ';';

        // Save the executable
        if (std::filesystem::exists(path)) {
            LOG_INFO(Debugger, "Symbol file {} already exists, skipping",
                     module_.name);
            continue;
        }

        // Load the executable
        auto file = debugger.getExecutables().at(module_.name);
        auto stream = file->open(horizon::filesystem::FileOpenFlags::Read);

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

void GdbServer::sendPacket(std::string_view data) const {
    ASSERT_DEBUG(client_socket != -1, Debugger, "Client socket is not valid");

    u8 checksum = 0;
    for (char c : data)
        checksum += c;

    std::string packet = fmt::format("${}#{:02x}", data, checksum);
    // TODO: debug
    LOG_INFO(Debugger, "SENDING: {}", packet);
    send(client_socket, packet.data(), packet.size(), 0);
}

void GdbServer::sendStatus(char status) const {
    if (!do_ack)
        return;

    ASSERT_DEBUG(client_socket != -1, Debugger, "Client socket is not valid");
    send(client_socket, &status, 1, 0);
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

Signal GdbServer::signalFromHaltReason(hw::tegra_x1::cpu::HaltReason reason) {
    switch (reason) {
    case hw::tegra_x1::cpu::HaltReason::Breakpoint:
    case hw::tegra_x1::cpu::HaltReason::BrkInstruction:
    case hw::tegra_x1::cpu::HaltReason::StepComplete:
        return Signal::SigTrap;
    case hw::tegra_x1::cpu::HaltReason::Fault:
        return Signal::SigSegv;
    }
}

Target& GdbServer::getTarget() const { return debugger.getTarget(); }

std::optional<std::string> GdbServer::getThreadStatus(thread_id_t id,
                                                      Signal signal) {
    auto stopped = getTarget().tryAcquireStoppedThread(id);
    if (!stopped)
        return std::nullopt;

    const auto& state = stopped->getState();
    return fmt::format(
        "T{:02x}{:02x}:{};{:02x}:{};{:02x}:{};thread:{:x};",
        static_cast<u8>(signal), PC_REGISTER, numberToHex(state.pc),
        SP_REGISTER, numberToHex(state.sp), LR_REGISTER, numberToHex(state.lr),
        id);
}

std::string GdbServer::pageFromBuffer(std::string_view buffer,
                                      std::string_view page) {
    const auto comma_pos = page.find(',');
    const auto offset_str = page.substr(0, comma_pos);
    const auto size_str = page.substr(comma_pos + 1);
    ZTD_ASSIGN_OR(const auto offset, fromChars<u64>(offset_str, 16),
                  LOG_FATAL(Debugger, "Invalid offset {}", offset_str));
    ZTD_ASSIGN_OR(const auto size, fromChars<u64>(size_str, 16),
                  LOG_FATAL(Debugger, "Invalid size {}", size_str));

    if (offset + size <= buffer.size())
        return fmt::format("m{}", buffer.substr(offset, size));
    else
        return fmt::format("l{}", buffer.substr(offset));
}

} // namespace hydra::debugger
