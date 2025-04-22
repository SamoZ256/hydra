#pragma once

#include "common/common.hpp"

namespace Hydra::Horizon::Kernel {

// TODO: how does this work?
constexpr uptr ADDRESS_SPACE_START = 0x10000000;
constexpr uptr ADDRESS_SPACE_END = 0x200000000;
constexpr usize ADDRESS_SPACE_SIZE = ADDRESS_SPACE_END - ADDRESS_SPACE_START;

constexpr uptr STACK_REGION_BASE = 0x10000000;
constexpr usize STACK_REGION_SIZE = 0x10000000;
constexpr usize DEFAULT_STACK_MEM_SIZE = 0x2000000;

constexpr uptr ALIAS_REGION_BASE = 0x30000000;
constexpr usize ALIAS_REGION_SIZE = STACK_REGION_SIZE;

constexpr uptr TLS_REGION_BASE = 0x20000000;
constexpr usize TLS_REGION_SIZE = 0x10000000;
constexpr usize DEFAULT_TLS_MEM_SIZE = 0x20000;

constexpr uptr HEAP_REGION_BASE = 0x100000000;
constexpr usize HEAP_REGION_SIZE = 0x100000000;
constexpr usize DEFAULT_HEAP_MEM_SIZE = 0x1000000;
constexpr usize HEAP_MEM_ALIGNMENT = 0x200000;

// TODO: correct?
constexpr i64 INFINITE_TIMEOUT = -1;

enum class Module {
    Svc = 1,
    Fs = 2,
    Os = 3,
    Htcs = 4,
    Ncm = 5,
    Dd = 6,
    Osdbg = 7,
    Lr = 8,
    Ldr = 9,
    Sf = 10,
    SfHipc = 11,
    Tma = 12,
    Dmnt = 13,
    Gds = 14,
    Pm = 15,
    Ns = 16,
    Bsdsockets = 17,
    Htc = 18,
    Tsc = 19,
    Kvdb = 20,
    Sm = 21,
    Ro = 22,
    Gc = 23,
    Sdmmc = 24,
    Ovln = 25,
    Spl = 26,
    Socket = 27,
    Unknown28 = 28,
    Htclow = 29,
    Ddsf = 30,
    Htcfs = 31,
    Async = 32,
    Util = 33,
    Unknown34 = 34,
    Tipc = 35,
    Unknown36 = 36,
    Anif = 37,
    Unknown38 = 38,
    Crt = 39,
    Eth = 100,
    I2c = 101,
    Gpio = 102,
    Uart = 103,
    Cpad = 104,
    Settings = 105,
    Ftm = 106,
    Wlan = 107,
    Xcd = 108,
    Tmp451 = 109,
    Nifm = 110,
    Codec = 111,
    Lsm6ds3 = 112,
    Bluetooth = 113,
    Vi = 114,
    Nfp = 115,
    Time = 116,
    Fgm = 117,
    Oe = 118,
    Bh1730fvc = 119,
    Pcie = 120,
    Friends = 121,
    Bcat = 122,
    Ssl = 123,
    Account = 124,
    News = 125,
    Mii = 126,
    Nfc = 127,
    Am = 128,
    Prepo = 129,
    Ahid = 130,
    Applet = 131,
    Ae = 132,
    Pcv = 133,
    UsbPd = 134,
    Bpc = 135,
    Psm = 136,
    Nim = 137,
    Psc = 138,
    Tc = 139,
    Usb = 140,
    Nsd = 141,
    Pctl = 142,
    Btm = 143,
    La = 144,
    Es = 145,
    Ngc = 146,
    Erpt = 147,
    Apm = 148,
    Cec = 149,
    Profiler = 150,
    Eupld = 151,
    Lidbe = 152,
    Audio = 153,
    Npns = 154,
    Http = 155,
    Idle = 156,
    Arp = 157,
    Updater = 158,
    Swkbd = 159,
    Netdiag = 160,
    NfcMifare = 161,
    Err = 162,
    Fatal = 163,
    Ec = 164,
    Spsm = 165,
    Aoc = 166,
    Bgtc = 167,
    Creport = 168,
    Sasbus = 169,
    Pl = 170,
    Cdmsc = 171,
    Audioctrl = 172,
    Lbl = 173,
    Unknown174 = 174,
    Jit = 175,
    Hdcp = 176,
    Omm = 177,
    Pdm = 178,
    Olsc = 179,
    Srepo = 180,
    Dauth = 181,
    Stdfu = 182,
    Dbg = 183,
    Cdacm = 184,
    Tcap = 185,
    Dhcps = 186,
    Spi = 187,
    Avm = 188,
    Pwm = 189,
    Dnsserver = 190,
    Rtc = 191,
    Regulator = 192,
    Led = 193,
    HtcTool = 194,
    Sio = 195,
    Pcm = 196,
    Clkrst = 197,
    Powctl = 198,
    Hiddriver = 199,
    Dma = 200,
    Audio_Old = 201,
    Hid = 202,
    Ldn = 203,
    Cs = 204,
    Irsensor = 205,
    Capsrv = 206,
    Mm = 207,
    Manu = 208,
    Atk = 209,
    Web = 210,
    Lcs = 211,
    Grc = 212,
    Repair = 213,
    Album = 214,
    Rid = 215,
    Migration = 216,
    MigrationIdc = 217,
    Hidbus = 218,
    Ens = 219,
    Nd = 220,
    Ndd = 221,
    Toycon = 222,
    Websocket = 223,
    Socketio = 224,
    Unknown225 = 225,
    Unknown226 = 226,
    Dcdmtp = 227,
    Pgl = 228,
    Notification = 229,
    Ins = 230,
    Lp2p = 231,
    Rcd = 232,
    Icm40607 = 233,
    Unknown234 = 234,
    Prc = 235,
    Unknown236 = 236,
    Bridgectrl = 237,
    ErrContext = 238,
    Mnpp = 239,
    Hshl = 240,
    Ringcon = 241,
    Capmtp = 242,
    Unknown243 = 243,
    Dp2hdmi = 244,
    Cradle = 245,
    Sprofile = 246,
    Unknown247 = 247,
    Icm42607p = 248,
    Unknown249 = 249,
    Ndrm = 250,
    Fst2 = 251,
    Unknown252 = 252,
    Ts = 253,
    Unknown254 = 254,
    Unknown255 = 255,
    Unknown300 = 300,
    Unknown301 = 301,
    Unknown302 = 302,
    Unknown303 = 303,
    Unknown304 = 304,
    Unknown305 = 305,
    Unknown306 = 306,
    Nex = 307,
    Unknown308 = 308,
    Unknown309 = 309,
    Unknown310 = 310,
    Unknown311 = 311,
    Unknown312 = 312,
    Unknown313 = 313,
    Unknown314 = 314,
    Unknown315 = 315,
    Unknown316 = 316,
    Unknown317 = 317,
    Unknown318 = 318,
    Unknown319 = 319,
    Unknown320 = 320,
    Unknown321 = 321,
    Npln = 322,
    Tspm = 499,
    Devmenu = 500
};

// From https://github.com/switchbrew/libnx
enum class Error {
    OutOfSessions = 7,
    InvalidCapabilityDescriptor = 14,
    NotImplemented = 33,
    ThreadTerminating = 59,
    OutOfDebugEvents = 70,
    InvalidSize = 101,
    InvalidAddress = 102,
    ResourceExhausted = 103,
    OutOfMemory = 104,
    OutOfHandles = 105,
    InvalidMemoryState = 106,
    InvalidMemoryPermissions = 108,
    InvalidMemoryRange = 110,
    InvalidPriority = 112,
    InvalidCoreId = 113,
    InvalidHandle = 114,
    InvalidUserBuffer = 115,
    InvalidCombination = 116,
    TimedOut = 117,
    Cancelled = 118,
    OutOfRange = 119,
    InvalidEnumValue = 120,
    NotFound = 121,
    AlreadyExists = 122,
    ConnectionClosed = 123,
    UnhandledUserInterrupt = 124,
    InvalidState = 125,
    ReservedValue = 126,
    InvalidHwBreakpoint = 127,
    FatalUserException = 128,
    OwnedByAnotherProcess = 129,
    ConnectionRefused = 131,
    OutOfResource = 132,
    IpcMapFailed = 259,
    IpcCmdbufTooSmall = 260,
    NotDebugged = 520,
};

typedef u32 Result;

#define MAKE_RESULT(module, description)                                       \
    (((static_cast<u32>(::Hydra::Horizon::Kernel::Module::module) & 0x1FF)) |  \
     (static_cast<u32>(description) & 0x1FFF) << 9)

#define RESULT_SUCCESS 0

enum class MemoryType : u32 {
    Free = 0x00000000,
    Io = 0x00002001,
    Static = 0x00042002,
    Code = 0x00dc7e03,
    CodeData_1_0_0 = 0x01febd04,
    CodeData_4_0_0 = 0x03febd04,
    Normal_1_0_0 = 0x017ebd05,
    Normal_4_0_0 = 0x037ebd05,
    Shared = 0x00402006,
    Alias = 0x00482907,
    AliasCode = 0x00dd7e08,
    AliasCodeData_1_0_0 = 0x01ffbd09,
    AliasCodeData_4_0_0 = 0x03ffbd09,
    Ipc = 0x005c3c0a,
    Stack = 0x005c3c0b,
    ThreadLocal = 0x0040200c,
    Transfered = 0x015c3c0d,
    SharedTransfered = 0x005c380e,
    SharedCode = 0x0040380f,
    Inaccessible = 0x00000010,
    NonSecureIpc = 0x005c3811,
    NonDeviceIpc = 0x004c2812,
    Kernel = 0x00002013,
    GeneratedCode = 0x00402214,
    CodeOut = 0x00402015,
    Coverage = 0x00002016,
    Insecure = 0x05583817,
};

enum class MemoryAttribute : u32 {
    None = 0,
    Locked = BIT(0),
    IpcLocked = BIT(1),
    DeviceShared = BIT(2),
    Uncached = BIT(3),
};
ENABLE_ENUM_BITMASK_OPERATORS(MemoryAttribute)

enum class MemoryPermission : u32 {
    None = 0x0,
    Read = BIT(0),
    Write = BIT(1),
    Execute = BIT(2),
    ReadWrite = Read | Write,
    ReadExecute = Read | Execute,
    ReadWriteExecute = Read | Write | Execute,
    DontCare = BIT(28),
};
ENABLE_ENUM_BITMASK_OPERATORS(MemoryPermission)

struct MemoryState {
    MemoryType type;
    MemoryAttribute attr;
    MemoryPermission perm;

    bool operator==(const MemoryState other) const {
        return type == other.type && attr == other.attr && perm == other.perm;
    }
};

struct MemoryInfo {
    u64 addr;
    u64 size;
    MemoryState state;
    u32 ipc_ref_count;    // TODO: what
    u32 device_ref_count; // TODO: what
    u32 padding = 0;
};

enum class BreakReasonType {
    Panic,
    Assert,
    User,
    PreLoadDll,
    PostLoadDll,
    PreUnloadDll,
    PostUnloadDll,
    CppException,
};

struct BreakReason {
    BreakReasonType type;
    bool notification_only;

    BreakReason(u64 reg) {
        notification_only = reg & 0x80000000;
        type = static_cast<BreakReasonType>(reg & 0x7FFFFFFF);
    }
};

// From https://github.com/switchbrew/libnx
enum class InfoType : u32 {
    CoreMask = 0,            ///< Bitmask of allowed Core IDs.
    PriorityMask = 1,        ///< Bitmask of allowed Thread Priorities.
    AliasRegionAddress = 2,  ///< Base of the Alias memory region.
    AliasRegionSize = 3,     ///< Size of the Alias memory region.
    HeapRegionAddress = 4,   ///< Base of the Heap memory region.
    HeapRegionSize = 5,      ///< Size of the Heap memory region.
    TotalMemorySize = 6,     ///< Total amount of memory available for process.
    UsedMemorySize = 7,      ///< Amount of memory currently used by process.
    DebuggerAttached = 8,    ///< Whether current process is being debugged.
    ResourceLimit = 9,       ///< Current process's resource limit handle.
    IdleTickCount = 10,      ///< Number of idle ticks on CPU.
    RandomEntropy = 11,      ///< [2.0.0+] Random entropy for current process.
    AslrRegionAddress = 12,  ///< [2.0.0+] Base of the process's address space.
    AslrRegionSize = 13,     ///< [2.0.0+] Size of the process's address space.
    StackRegionAddress = 14, ///< [2.0.0+] Base of the Stack memory region.
    StackRegionSize = 15,    ///< [2.0.0+] Size of the Stack memory region.
    SystemResourceSizeTotal =
        16, ///< [3.0.0+] Total memory allocated for process memory management.
    SystemResourceSizeUsed = 17, ///< [3.0.0+] Amount of memory currently used
                                 ///< by process memory management.
    ProgramId = 18,              ///< [3.0.0+] Program ID for the process.
    InitialProcessIdRange = 19,  ///< [4.0.0-4.1.0] Min/max initial process IDs.
    UserExceptionContextAddress = 20, ///< [5.0.0+] Address of the process's
                                      ///< exception context (for break).
    TotalNonSystemMemorySize =
        21, ///< [6.0.0+] Total amount of memory available for process,
            ///< excluding that for process memory management.
    UsedNonSystemMemorySize =
        22, ///< [6.0.0+] Amount of memory used by process, excluding that for
            ///< process memory management.
    IsApplication =
        23, ///< [9.0.0+] Whether the specified process is an Application.
    FreeThreadCount = 24, ///< [11.0.0+] The number of free threads available to
                          ///< the process's resource limit.
    ThreadTickCount = 25, ///< [13.0.0+] Number of ticks spent on thread.
    IsSvcPermitted =
        26, ///< [14.0.0+] Does process have access to SVC (only usable with
            ///< \ref svcSynchronizePreemptionState at present).
    IoRegionHint =
        27, ///< [16.0.0+] Low bits of the physical address for a KIoRegion.
    AliasRegionExtraSize =
        28, ///< [18.0.0+] Extra size added to the reserved region.

    TransferMemoryHint = 34, ///< [19.0.0+] Low bits of the process
                             ///< address for a KTransferMemory.

    ThreadTickCountDeprecated =
        0xF0000002, ///< [1.0.0-12.1.0] Number of ticks spent on thread.
};

// From https://github.com/switchbrew/libnx
enum class SystemInfoType {
    TotalPhysicalMemorySize = 0, ///< Total amount of DRAM available to system.
    UsedPhysicalMemorySize = 1,  ///< Current amount of DRAM used by system.
    InitialProcessIdRange = 2,   ///< Min/max initial process IDs.
};

// TODO: idle tick count -1, {current coreid} (probably the same logic as thread
// tick count)
// TODO: random entropy 0 - 3
constexpr u32 INFO_SUB_TYPE_INITIAL_PROCESS_ID_RANGE_LOWER_BOUND = 0;
constexpr u32 INFO_SUB_TYPE_INITIAL_PROCESS_ID_RANGE_UPPER_BOUND = 1;
constexpr u32 INFO_SUB_TYPE_THREAD_TICK_COUNT_CORE0 = 0;
constexpr u32 INFO_SUB_TYPE_THREAD_TICK_COUNT_CORE1 = 1;
constexpr u32 INFO_SUB_TYPE_THREAD_TICK_COUNT_CORE2 = 2;
constexpr u32 INFO_SUB_TYPE_THREAD_TICK_COUNT_CORE3 = 3;
constexpr u32 INFO_SUB_TYPE_THREAD_TICK_COUNT_ALL = 0xFFFFFFFF;

class Handle {
  public:
    virtual ~Handle() = default;
};

} // namespace Hydra::Horizon::Kernel

ENABLE_ENUM_FORMATTING(
    Hydra::Horizon::Kernel::MemoryType, Free, "free", Io, "io", Static,
    "static", Code, "code", CodeData_1_0_0, "code data 1.0.0", CodeData_4_0_0,
    "code data 4.0.0", Normal_1_0_0, "normal 1.0.0", Normal_4_0_0,
    "normal 4.0.0", Shared, "shared", Alias, "alias", AliasCode, "alias code",
    AliasCodeData_1_0_0, "alias code data 1.0.0", AliasCodeData_4_0_0,
    "alias code data 4.0.0", Ipc, "ipc", Stack, "stack", ThreadLocal,
    "thread local", Transfered, "transfered", SharedTransfered,
    "shared transfered", SharedCode, "shared code", Inaccessible,
    "inaccessible", NonSecureIpc, "non secure ipc", NonDeviceIpc,
    "non device ipc", Kernel, "kernel", GeneratedCode, "generated code",
    CodeOut, "code out", Coverage, "coverage", Insecure, "insecure")

ENABLE_ENUM_FLAGS_FORMATTING(Hydra::Horizon::Kernel::MemoryPermission, Read,
                             "read", Write, "write", Execute, "execute",
                             DontCare, "don't care")

ENABLE_ENUM_FORMATTING(Hydra::Horizon::Kernel::BreakReasonType, Panic, "panic",
                       Assert, "assert", User, "user", PreLoadDll,
                       "pre load dll", PostLoadDll, "post load dll",
                       PreUnloadDll, "pre unload dll", PostUnloadDll,
                       "post unload dll", CppException, "cpp exception")

ENABLE_ENUM_FORMATTING(
    Hydra::Horizon::Kernel::InfoType, CoreMask, "core mask", PriorityMask,
    "priority mask", AliasRegionAddress, "alias region address",
    AliasRegionSize, "alias region size", HeapRegionAddress,
    "heap region address", HeapRegionSize, "heap region size", TotalMemorySize,
    "total memory size", UsedMemorySize, "used memory size", DebuggerAttached,
    "debugger attached", ResourceLimit, "resource limit", IdleTickCount,
    "idle tick count", RandomEntropy, "random entropy", AslrRegionAddress,
    "aslr region address", AslrRegionSize, "aslr region size",
    StackRegionAddress, "stack region address", StackRegionSize,
    "stack region size", SystemResourceSizeTotal, "system resource size total",
    SystemResourceSizeUsed, "system resource size used", ProgramId,
    "program id", InitialProcessIdRange, "initial process id range",
    UserExceptionContextAddress, "user exception context address",
    TotalNonSystemMemorySize, "total non-system memory size",
    UsedNonSystemMemorySize, "used non-system memory size", IsApplication,
    "is application", FreeThreadCount, "free thread count", ThreadTickCount,
    "thread tick count", IsSvcPermitted, "is svc permitted", IoRegionHint,
    "io region hint", AliasRegionExtraSize, "alias region extra size",
    TransferMemoryHint, "transfer memory hint", ThreadTickCountDeprecated,
    "thread tick count deprecated")

ENABLE_ENUM_FORMATTING(Hydra::Horizon::Kernel::SystemInfoType,
                       TotalPhysicalMemorySize, "total physical memory size",
                       UsedPhysicalMemorySize, "used physical memory size",
                       InitialProcessIdRange, "initial process id range")

#include "common/logging/log.hpp"
