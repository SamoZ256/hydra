#include "core/debugger/gdb_server.hpp"

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <nx2elf.h>

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/filesystem/file_base.hpp"
#include "core/horizon/kernel/guest_thread.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

#define GET_THREAD_ID(thread) reinterpret_cast<u64>(thread)
#define GET_THREAD_FROM_ID(id)                                                 \
    reinterpret_cast<horizon::kernel::GuestThread*>(id)

namespace hydra::debugger {

namespace {

constexpr char GDB_START = '$';
constexpr char GDB_END = '#';
constexpr char GDB_ACK = '+';
constexpr char GDB_NACK = '-';

constexpr char GDB_INT3 = 0x03;
constexpr int GDB_SIGTRAP = 5;

constexpr char GDB_OK[] = "OK";
constexpr char GDB_ERROR[] = "E01";
constexpr char GDB_EMPTY[] = "";

constexpr u32 FP_REGISTER = 29;
constexpr u32 LR_REGISTER = 30;
constexpr u32 SP_REGISTER = 31;
constexpr u32 PC_REGISTER = 32;
constexpr u32 PSTATE_REGISTER = 33;
constexpr u32 Q0_REGISTER = 34;
constexpr u32 FPSR_REGISTER = 66;
constexpr u32 FPCR_REGISTER = 67;

constexpr u32 BRK = 0xd4200000;

enum class BreakpointType {
    Software = 0,
    Hardware = 1,
    WriteWatch = 2,
    ReadWatch = 3,
    AccessWatch = 4,
};

template <typename T>
std::string number_to_hex(T value) {
    const auto ptr = reinterpret_cast<const u8*>(&value);
    std::string hex;
    hex.reserve(sizeof(T) * 2);
    for (size_t i = 0; i < sizeof(T); ++i)
        hex += fmt::format("{:02x}", ptr[i]);

    return hex;
}

template <typename T>
T hex_to_number(std::string_view hex) {
    T value = 0;
    for (size_t i = 0; i < sizeof(T); ++i)
        value |=
            static_cast<T>(std::stoi(hex.substr(i * 2, 2).data(), nullptr, 16))
            << (i * 8);

    return value;
}

std::string_view get_target_xml_aarch64() {
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
    const u16 port = CONFIG_INSTANCE.GetGdbPort();

    // Create the socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        LOG_ERROR(Debugger, "Failed to create GDB socket");
        return;
    }

    // Set the socket to reuse address
    i32 opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt,
               sizeof(opt));

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
    SetNonBlocking(server_socket);

    // Create server thread
    server_thread = std::thread(&GdbServer::ServerLoop, this);

    LOG_INFO(Debugger, "GDB server started on port {}", port);

    // Thread ID
    ASSERT(!debugger.threads.empty(), Debugger, "No active thread");
    crnt_thread = debugger.threads.begin()->second.guest_thread;
}

GdbServer::~GdbServer() {
    running = false;
    server_thread.join();
    close(server_socket);
}

void GdbServer::NotifySupervisorPaused(horizon::kernel::GuestThread* thread) {
    // TODO: don't send packets from other threads
    SendPacket(GetThreadStatus(thread, GDB_SIGTRAP));
}

void GdbServer::BreakpointHit(horizon::kernel::GuestThread* thread) {
    while (breakpoint_hit.exchange(true))
        thread->ProcessMessages();

    // We got the lock
    breakpoint_thread = thread;

    for (const auto& [_, thread] : debugger.threads)
        thread.guest_thread->SupervisorPause();

    NotifySupervisorPaused(thread);
}

void GdbServer::CloseClientSocket() {
    ASSERT(client_socket != -1, Debugger, "Client socket is not open");
    close(client_socket);
    client_socket = -1;
    LOG_INFO(Debugger, "GDB client disconnected");
}

void GdbServer::ServerLoop() {
    GET_CURRENT_PROCESS_DEBUGGER().RegisterThisThread("GDB server");
    while (running) {
        Poll();
        // TODO: is the delay necessary?
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    GET_CURRENT_PROCESS_DEBUGGER().UnregisterThisThread();
}

void GdbServer::Poll() {
    if (client_socket == -1) {
        sockaddr_in client_addr{};
        socklen_t addr_len = sizeof(client_addr);
        i32 new_client =
            accept(server_socket, (sockaddr*)&client_addr, &addr_len);

        if (new_client != -1) {
            client_socket = new_client;
            SetNonBlocking(client_socket);

            // Pause all threads
            // TODO: helper function?
            for (const auto& [_, thread] : debugger.threads)
                thread.guest_thread->SupervisorPause();
        }
    } else {
        char buffer[1024];
        ssize_t bytes_read = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_read > 0) {
            receive_buffer += std::string_view(buffer, bytes_read);
            ProcessPackets();
        } else if (bytes_read == 0) {
            CloseClientSocket();
            // TODO: also resume all threads?
        }
    }
}

void GdbServer::ProcessPackets() {
    size_t str_start = 0;
    while (true) {
        const auto start = receive_buffer.find(GDB_START, str_start);
        if (start == std::string::npos)
            break;

        const auto end = receive_buffer.find(GDB_END, start);
        if (end == std::string::npos)
            break;

        std::string command = receive_buffer.substr(start + 1, end - start - 1);
        if (end + 2 < receive_buffer.size()) {
            SendStatus(GDB_ACK);
            HandleCommand(command);
        } else {
            SendStatus(GDB_NACK);
            break;
        }

        str_start = end + 3;
    }

    if (str_start > 0)
        receive_buffer = receive_buffer.substr(str_start);
}

void GdbServer::HandleCommand(std::string_view command) {
    // TODO: only log in debug
    LOG_INFO(Debugger, "COMMAND: {}", command);

    if (command.starts_with("vCont")) {
        HandleVCont(command.substr(5));
        return;
    }

    const auto body = command.substr(1);
    switch (command[0]) {
    case 'Q':
    case 'q':
        HandleQuery(body);
        break;
    case 'k':
        debugger.process->Stop();
        CloseClientSocket();
        break;
    case 'H':
        HandleSetActiveThread(body);
        break;
    case '?':
        HandleThreadStatus();
        break;
    case 'p':
        HandleRegRead(body);
        break;
    case 'm':
        HandleMemRead(body);
        break;
    case 'c':
        // Resume all threads
        for (const auto& [_, thread] : debugger.threads)
            thread.guest_thread->SupervisorResume();

        breakpoint_thread = nullptr;
        breakpoint_hit = false;
        break;
    case 'Z':
        HandleInsertBreakpoint(body);
        break;
    case 'z':
        HandleRemoveBreakpoint(body);
        break;
    default:
        LOG_WARN(Debugger, "Unhandled GDB command: {}", command);
        SendPacket(GDB_EMPTY);
        break;
    }
}

void GdbServer::HandleVCont(std::string_view command) {
    if (command == "?") {
        SendPacket("vCont;c;C;s;S");
        return;
    }

    horizon::kernel::GuestThread* thread = nullptr;
    bool lock_execution = true; // TODO: what is this?

    size_t pos = 1;
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
            thread = GET_THREAD_FROM_ID(
                std::stoull(thread_id_str.data(), nullptr, 16));
            break;
        }
        default:
            LOG_WARN(Debugger, "Unhandled GDB vCont entry: {}", entry[0]);
            SendPacket(GDB_EMPTY);
            break;
        }
    } while (pos != std::string::npos);

    // TODO: how does this work?
    if (thread) {
        // TODO: implement
        ASSERT_DEBUG(lock_execution, Debugger,
                     "Non-locked execution not implemented");
        crnt_thread = thread;

        thread->GetThread()->SingleStep();
        if (CPU_INSTANCE.GetFeatures().supports_synchronous_single_step) {
            // Single-stepping already finished
            NotifySupervisorPaused(thread);
        } else {
            // Resume the thread for asynchronous single-stepping
            thread->SupervisorResume();
        }
    } else {
        for (const auto& [_, thread] : debugger.threads)
            thread.guest_thread->SupervisorResume();
    }
}

void GdbServer::HandleQuery(std::string_view command) {
    if (command == "StartNoAckMode") {
        do_ack = false;
        SendPacket(GDB_OK);
    } else if (command.starts_with("Supported")) {
        // TODO: why packet size 4000?
        SendPacket("PacketSize=4000;qXfer:features:read+;qXfer:threads:read+;"
                   "qXfer:libraries:read+;"
                   "vContSupported+;QStartNoAckMode+");
    } else if (command.starts_with("fThreadInfo")) { // TODO: ==?
        std::vector<std::string> thread_ids;
        thread_ids.reserve(debugger.threads.size());
        for (const auto& [_, thread] : debugger.threads)
            thread_ids.push_back(
                fmt::format("{:x}", GET_THREAD_ID(thread.guest_thread)));
        SendPacket(fmt::format("m{}", fmt::join(thread_ids, ",")));
    } else if (command.starts_with("sThreadInfo")) { // TODO: ==?
        // TODO: why?
        SendPacket("l");
    } else if (command.starts_with("Xfer:features:read:target.xml:")) {
        const auto& target_xml = get_target_xml_aarch64();
        SendPacket(PageFromBuffer(target_xml, command.substr(30)));
    } else if (command.starts_with("Xfer:libraries:read::")) {
        std::string output = R"(<?xml version="1.0"?><library-list>)";
        for (const auto& symbol : debugger.module_table.GetSymbols()) {
            // TODO: what's the purpose of this?
            // TODO: number_to_hex?
            output += fmt::format(
                R"(<library name="{}"><segment address="{:#x}"/></library>)",
                symbol.name, symbol.guest_mem_range.begin);
        }
        output += "</library-list>";
        SendPacket(PageFromBuffer(output, command.substr(21)));
    } else if (command.starts_with("Rcmd")) {
        HandleRcmd(command.substr(5));
    } else {
        LOG_WARN(Debugger, "Unhandled GDB query: {}", command);
        SendPacket(GDB_EMPTY);
    }
}

void GdbServer::HandleSetActiveThread(std::string_view command) {
    const auto thread =
        GET_THREAD_FROM_ID(std::stoull(command.substr(1).data(), nullptr, 16));
    if (thread) {
        // TODO: check if thread is valid
        // if (debugger.threads.contains(thread)) {
        //    crnt_thread = thread;
        //    SendPacket(GDB_OK);
        //} else {
        //    SendPacket(GDB_ERROR);
        //}
        crnt_thread = thread;
        SendPacket(GDB_OK);
    }
}

void GdbServer::HandleThreadStatus() {
    SendPacket(GetThreadStatus(crnt_thread, GDB_SIGTRAP));
}

void GdbServer::HandleRegRead(std::string_view command) {
    const auto id = static_cast<u32>(std::stoul(command.data(), nullptr, 16));
    SendPacket(ReadReg(id));
}

void GdbServer::HandleMemRead(std::string_view command) {
    const auto comma_pos = command.find(',');
    const auto addr =
        std::stoull(command.substr(0, comma_pos).data(), nullptr, 16);
    const auto size =
        std::stoull(command.substr(comma_pos + 1).data(), nullptr, 16);

    std::string output;
    output.reserve(size * 2);
    const auto mmu = debugger.process->GetMmu();
    for (u64 i = 0; i < size; i++) {
        const auto crnt_addr = addr + i;

        // Check for replaced instructions
        const auto it = replaced_instructions.find(crnt_addr);
        if (it != replaced_instructions.end()) {
            const u8* inst = reinterpret_cast<const u8*>(&it->second);
            const auto count = std::min(size - i, 4ull);
            for (u32 j = 0; j < count; j++)
                output += fmt::format("{:02x}", inst[j]);
            i += count - 1;
            continue;
        }

        u8 value;
        if (!mmu->TryRead(crnt_addr, value)) {
            SendPacket(GDB_ERROR);
            return;
        }
        output += fmt::format("{:02x}", value);
    }
    SendPacket(output);
}

void GdbServer::HandleInsertBreakpoint(std::string_view command) {
    const auto addr_pos = command.find(',') + 1;
    const auto size_pos = command.find(',', addr_pos) + 1;

    const auto type =
        static_cast<BreakpointType>(std::stoul(command.data(), nullptr, 16));
    const auto addr = std::stoull(command.substr(addr_pos).data(), nullptr, 16);
    const auto size = std::stoull(command.substr(size_pos).data(), nullptr, 16);

    switch (type) {
    case BreakpointType::Software:
        ASSERT_DEBUG(size == 4, Debugger,
                     "Invalid software breakpoint size 0x{:x}", size);

        if (CPU_INSTANCE.GetFeatures().supports_native_breakpoints) {
            for (const auto& [_, thread] : debugger.threads)
                thread.guest_thread->GetThread()->InsertBreakpoint(addr);
        } else {
            const auto mmu = debugger.process->GetMmu();
            replaced_instructions.insert({addr, mmu->Read<u32>(addr)});
            mmu->Write(addr, BRK);
            NotifyMemoryChanged(range<vaddr_t>(addr, 4));
        }

        SendPacket(GDB_OK);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Debugger, "Breakpoint type {}", type);
        SendPacket(GDB_ERROR);
        break;
    }
}

void GdbServer::HandleRemoveBreakpoint(std::string_view command) {
    const auto addr_pos = command.find(',') + 1;
    const auto size_pos = command.find(',', addr_pos) + 1;

    const auto type =
        static_cast<BreakpointType>(std::stoul(command.data(), nullptr, 16));
    const auto addr = std::stoull(command.substr(addr_pos).data(), nullptr, 16);
    const auto size = std::stoull(command.substr(size_pos).data(), nullptr, 16);

    switch (type) {
    case BreakpointType::Software:
        ASSERT_DEBUG(size == 4, Debugger,
                     "Invalid software breakpoint size 0x{:x}", size);

        if (CPU_INSTANCE.GetFeatures().supports_native_breakpoints) {
            for (const auto& [_, thread] : debugger.threads)
                thread.guest_thread->GetThread()->RemoveBreakpoint(addr);
        } else {
            const auto mmu = debugger.process->GetMmu();
            auto it = replaced_instructions.find(addr);
            ASSERT(it != replaced_instructions.end(), Debugger,
                   "Breakpoint not found at address {:#x}", addr);
            mmu->Write(addr, it->second);
            NotifyMemoryChanged(range<vaddr_t>(addr, 4));
            replaced_instructions.erase(it);
        }

        SendPacket(GDB_OK);
        break;
    default:
        LOG_NOT_IMPLEMENTED(Debugger, "Breakpoint type {}", type);
        SendPacket(GDB_ERROR);
        break;
    }
}

void GdbServer::HandleRcmd(std::string_view cmd) {
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
        HandleGetExecutables();
    } else {
        LOG_WARN(Debugger, "Unhandled GDB RCMD: {}", command);
        SendPacket(GDB_EMPTY);
    }
}

void GdbServer::HandleGetExecutables() {
    std::string dir_path = fmt::format("/tmp/hydra/dump/executables/{:016x}",
                                       debugger.process->GetTitleID());
    std::filesystem::create_directories(dir_path);

    std::string output;
    for (u32 i = 0; i < debugger.GetModuleTable().GetSymbols().size(); i++) {
        const auto& module_ = debugger.GetModuleTable().GetSymbols()[i];
        std::string path = fmt::format("{}/{}.elf", dir_path, module_.name);

        // Output
        output +=
            fmt::format("\"{}\":{:#x}", path, module_.guest_mem_range.begin);
        if (i < debugger.GetModuleTable().GetSymbols().size() - 1)
            output += ";";

        // Save the executable
        if (std::filesystem::exists(path)) {
            LOG_INFO(Debugger, "Symbol file {} already exists, skipping",
                     module_.name);
            continue;
        }

        // Load the executable
        auto file = debugger.executables.at(module_.name);
        auto stream = file->Open(horizon::filesystem::FileOpenFlags::Read);
        auto reader = stream.CreateReader();

        std::vector<u8> data(reader.GetSize());
        reader.ReadWhole(data.data());

        file->Close(stream);

        // Convert to ELF
        NsoFile nso;
        nso.Load(data);
        nso.WriteElf(path);
    }

    SendPacket(output);
}

void GdbServer::SendPacket(std::string_view data) {
    ASSERT_DEBUG(client_socket != -1, Debugger, "Client socket is not valid");

    u8 checksum = 0;
    for (char c : data)
        checksum += c;

    std::string packet = fmt::format("${}#{:02x}", data, checksum);
    // TODO: debug
    LOG_INFO(Debugger, "SENDING: {}", packet);
    send(client_socket, packet.data(), packet.size(), 0);
}

void GdbServer::SendStatus(char status) {
    if (!do_ack)
        return;

    ASSERT_DEBUG(client_socket != -1, Debugger, "Client socket is not valid");
    send(client_socket, &status, 1, 0);
}

void GdbServer::SetNonBlocking(i32 socket) {
    i32 flags = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

std::string GdbServer::ReadReg(u32 id) {
    const auto& state = crnt_thread->GetThread()->GetState();
    switch (id) {
    case 0 ... 28:
        return number_to_hex(state.r[id]);
    case 29:
        return number_to_hex(state.fp);
    case 30:
        return number_to_hex(state.lr);
    case 31:
        return number_to_hex(state.sp);
    case 32:
        return number_to_hex(state.pc);
    case 33:
        return number_to_hex(state.pstate);
    case 34 ... 65:
        return number_to_hex(state.v[id]);
    case 66:
        return number_to_hex(state.fpsr);
    case 67:
        return number_to_hex(state.fpcr);
    default:
        return GDB_ERROR;
    }
}

std::string GdbServer::GetThreadStatus(horizon::kernel::GuestThread* thread,
                                       u8 signal) {
    const auto& state = thread->GetThread()->GetState();
    return fmt::format("T{:02x}{:02x}:{};{:02x}:{};{:02x}:{};thread:{:x};",
                       signal, PC_REGISTER, state.pc, SP_REGISTER, state.sp,
                       LR_REGISTER, state.lr, GET_THREAD_ID(thread));
}

std::string GdbServer::PageFromBuffer(std::string_view buffer,
                                      std::string_view page) {
    const auto comma_pos = page.find(',');
    const auto offset =
        std::stoull(page.substr(0, comma_pos).data(), nullptr, 16);
    const auto size =
        std::stoull(page.substr(comma_pos + 1).data(), nullptr, 16);

    if (offset + size <= buffer.size())
        return fmt::format("m{}", buffer.substr(offset, size));
    else
        return fmt::format("l{}", buffer.substr(offset));
}

void GdbServer::NotifyMemoryChanged(range<vaddr_t> mem_range) {
    for (const auto& [_, thread] : debugger.threads)
        thread.guest_thread->GetThread()->NotifyMemoryChanged(mem_range);
}

} // namespace hydra::debugger
