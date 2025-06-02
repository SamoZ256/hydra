#pragma once

namespace hydra::horizon::kernel {

// TODO: how does this work?
constexpr uptr ADDRESS_SPACE_START = 0x10000000;
constexpr uptr ADDRESS_SPACE_END = 0x200000000;
constexpr usize ADDRESS_SPACE_SIZE = ADDRESS_SPACE_END - ADDRESS_SPACE_START;

constexpr uptr STACK_REGION_BASE = 0x10000000;
constexpr usize STACK_REGION_SIZE = 0x10000000;

constexpr uptr ALIAS_REGION_BASE = 0x30000000;
constexpr usize ALIAS_REGION_SIZE = 0x10000000;

constexpr uptr TLS_REGION_BASE = 0x20000000;
constexpr usize TLS_REGION_SIZE = 0x10000000;

constexpr uptr HEAP_REGION_BASE = 0x100000000;
constexpr usize HEAP_REGION_SIZE = 0x100000000;
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

typedef u32 result_t;

#define MAKE_RESULT(module, description)                                       \
    (((static_cast<u32>(::hydra::horizon::kernel::Module::module) & 0x1ff)) |  \
     (static_cast<u32>(description) & 0x1fff) << 9)

#define GET_RESULT_MODULE(result)                                              \
    static_cast<::hydra::horizon::kernel::Module>((result)&0x1ff)

#define GET_RESULT_DESCRIPTION(result) ((result) >> 9)

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
    CoreMask = 0,
    PriorityMask = 1,
    AliasRegionAddress = 2,
    AliasRegionSize = 3,
    HeapRegionAddress = 4,
    HeapRegionSize = 5,
    TotalMemorySize = 6,
    UsedMemorySize = 7,
    DebuggerAttached = 8,
    ResourceLimit = 9,
    IdleTickCount = 10,
    RandomEntropy = 11,
    AslrRegionAddress = 12,
    AslrRegionSize = 13,
    StackRegionAddress = 14,
    StackRegionSize = 15,
    TotalSystemResourceSize = 16,
    UsedSystemResourceSize = 17,
    ProgramId = 18,
    InitialProcessIdRange = 19,
    UserExceptionContextAddress = 20,
    TotalNonSystemMemorySize = 21,
    UsedNonSystemMemorySize = 22,
    IsApplication = 23,
    FreeThreadCount = 24,
    ThreadTickCount = 25,
    IsSvcPermitted = 26,
    IoRegionHint = 27,
    AliasRegionExtraSize = 28,
    TransferMemoryHint = 34,
    ThreadTickCountDeprecated = 0xF0000002,
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

enum class ThreadActivity {
    None = 0,
    Runnable = 1,
};

union CpuRegister {
    u64 x;
    u32 w;
    u32 r;
};

union FpuRegister {
    u128 v;
    double d;
    float s;
};

struct ThreadContext {
    CpuRegister cpu_gprs[29];
    u64 fp;
    u64 lr;
    u64 sp;
    CpuRegister pc;
    u32 psr;

    FpuRegister fpu_gprs[32];
    u32 fpcr;
    u32 fpsr;

    u64 tpidr;
};

class Handle {
  public:
    virtual ~Handle() = default;
};

} // namespace hydra::horizon::kernel

ENABLE_ENUM_FORMATTING(
    hydra::horizon::kernel::Module, Svc, "svc", Fs, "fs", Os, "os", Htcs,
    "htcs", Ncm, "ncm", Dd, "dd", Osdbg, "osdbg", Lr, "lr", Ldr, "ldr", Sf,
    "sf", SfHipc, "sf hipc", Tma, "tma", Dmnt, "dmnt", Gds, "gds", Pm, "pm", Ns,
    "ns", Bsdsockets, "bsd sockets", Htc, "htc", Tsc, "tsc", Kvdb, "kvdb", Sm,
    "sm", Ro, "ro", Gc, "gc", Sdmmc, "sdmmc", Ovln, "ovln", Spl, "spl", Socket,
    "socket", Unknown28, "unknown 28", Htclow, "htc low", Ddsf, "ddsf", Htcfs,
    "htc fs", Async, "async", Util, "util", Unknown34, "unknown 34", Tipc,
    "tipc", Unknown36, "unknown 36", Anif, "anif", Unknown38, "unknown 38", Crt,
    "crt", Eth, "eth", I2c, "i2c", Gpio, "gpio", Uart, "uart", Cpad, "cpad",
    Settings, "settings", Ftm, "ftm", Wlan, "wlan", Xcd, "xcd", Tmp451,
    "tmp451", Nifm, "nifm", Codec, "codec", Lsm6ds3, "lsm6ds3", Bluetooth,
    "bluetooth", Vi, "vi", Nfp, "nfp", Time, "time", Fgm, "fgm", Oe, "oe",
    Bh1730fvc, "bh1730fvc", Pcie, "pcie", Friends, "friends", Bcat, "bcat", Ssl,
    "ssl", Account, "account", News, "news", Mii, "mii", Nfc, "nfc", Am, "am",
    Prepo, "prepo", Ahid, "ahid", Applet, "applet", Ae, "ae", Pcv, "pcv", UsbPd,
    "usb pd", Bpc, "bpc", Psm, "psm", Nim, "nim", Psc, "psc", Tc, "tc", Usb,
    "usb", Nsd, "nsd", Pctl, "pctl", Btm, "btm", La, "la", Es, "es", Ngc, "ngc",
    Erpt, "erpt", Apm, "apm", Cec, "cec", Profiler, "profiler", Eupld, "eupld",
    Lidbe, "lidbe", Audio, "audio", Npns, "npns", Http, "http", Idle, "idle",
    Arp, "arp", Updater, "updater", Swkbd, "swkbd", Netdiag, "netdiag",
    NfcMifare, "nfc mifare", Err, "err", Fatal, "fatal", Ec, "ec", Spsm, "spsm",
    Aoc, "aoc", Bgtc, "bgtc", Creport, "creport", Sasbus, "sasbus", Pl, "pl",
    Cdmsc, "cdmsc", Audioctrl, "audioctrl", Lbl, "lbl", Unknown174,
    "unknown 174", Jit, "jit", Hdcp, "hdcp", Omm, "omm", Pdm, "pdm", Olsc,
    "olsc", Srepo, "srepo", Dauth, "dauth", Stdfu, "stdfu", Dbg, "dbg", Cdacm,
    "cdacm", Tcap, "tcap", Dhcps, "dhcps", Spi, "spi", Avm, "avm", Pwm, "pwm",
    Dnsserver, "dns server", Rtc, "rtc", Regulator, "regulator", Led, "led",
    HtcTool, "htc tool", Sio, "sio", Pcm, "pcm", Clkrst, "clkrst", Powctl,
    "powctl", Hiddriver, "hid driver", Dma, "dma", Audio_Old, "audio old", Hid,
    "hid", Ldn, "ldn", Cs, "cs", Irsensor, "ir sensor", Capsrv, "capsrv", Mm,
    "mm", Manu, "manu", Atk, "atk", Web, "web", Lcs, "lcs", Grc, "grc", Repair,
    "repair", Album, "album", Rid, "rid", Migration, "migration", MigrationIdc,
    "migration idc", Hidbus, "hidbus", Ens, "ens", Nd, "nd", Ndd, "ndd", Toycon,
    "toycon", Websocket, "websocket", Socketio, "socketio", Unknown225,
    "unknown 225", Unknown226, "unknown 226", Dcdmtp, "dcdmtp", Pgl, "pgl",
    Notification, "notification", Ins, "ins", Lp2p, "lp2p", Rcd, "rcd",
    Icm40607, "icm40607", Unknown234, "unknown 234", Prc, "prc", Unknown236,
    "unknown 236", Bridgectrl, "bridge ctrl", ErrContext, "err context", Mnpp,
    "mnpp", Hshl, "hshl", Ringcon, "ringcon", Capmtp, "capmtp", Unknown243,
    "unknown 243", Dp2hdmi, "dp2hdmi", Cradle, "cradle", Sprofile, "sprofile",
    Unknown247, "unknown 247", Icm42607p, "icm42607p", Unknown249,
    "unknown 249", Ndrm, "ndrm", Fst2, "fst2", Unknown252, "unknown 252", Ts,
    "ts", Unknown254, "unknown 254", Unknown255, "unknown 255", Unknown300,
    "unknown 300", Unknown301, "unknown 301", Unknown302, "unknown 302",
    Unknown303, "unknown 303", Unknown304, "unknown 304", Unknown305,
    "unknown 305", Unknown306, "unknown 306", Nex, "nex", Unknown308,
    "unknown 308", Unknown309, "unknown 309", Unknown310, "unknown 310",
    Unknown311, "unknown 311", Unknown312, "unknown 312", Unknown313,
    "unknown 313", Unknown314, "unknown 314", Unknown315, "unknown 315",
    Unknown316, "unknown 316", Unknown317, "unknown 317", Unknown318,
    "unknown 318", Unknown319, "unknown 319", Unknown320, "unknown 320",
    Unknown321, "unknown 321", Npln, "npln", Tspm, "tspm", Devmenu, "devmenu")

ENABLE_ENUM_FORMATTING(
    hydra::horizon::kernel::MemoryType, Free, "free", Io, "io", Static,
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

ENABLE_ENUM_FLAGS_FORMATTING(hydra::horizon::kernel::MemoryPermission, Read,
                             "read", Write, "write", Execute, "execute",
                             DontCare, "don't care")

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::BreakReasonType, Panic, "panic",
                       Assert, "assert", User, "user", PreLoadDll,
                       "pre load dll", PostLoadDll, "post load dll",
                       PreUnloadDll, "pre unload dll", PostUnloadDll,
                       "post unload dll", CppException, "cpp exception")

ENABLE_ENUM_FORMATTING(
    hydra::horizon::kernel::InfoType, CoreMask, "core mask", PriorityMask,
    "priority mask", AliasRegionAddress, "alias region address",
    AliasRegionSize, "alias region size", HeapRegionAddress,
    "heap region address", HeapRegionSize, "heap region size", TotalMemorySize,
    "total memory size", UsedMemorySize, "used memory size", DebuggerAttached,
    "debugger attached", ResourceLimit, "resource limit", IdleTickCount,
    "idle tick count", RandomEntropy, "random entropy", AslrRegionAddress,
    "aslr region address", AslrRegionSize, "aslr region size",
    StackRegionAddress, "stack region address", StackRegionSize,
    "stack region size", TotalSystemResourceSize, "total system resource size",
    UsedSystemResourceSize, "used system resource size", ProgramId,
    "program id", InitialProcessIdRange, "initial process id range",
    UserExceptionContextAddress, "user exception context address",
    TotalNonSystemMemorySize, "total non-system memory size",
    UsedNonSystemMemorySize, "used non-system memory size", IsApplication,
    "is application", FreeThreadCount, "free thread count", ThreadTickCount,
    "thread tick count", IsSvcPermitted, "is svc permitted", IoRegionHint,
    "io region hint", AliasRegionExtraSize, "alias region extra size",
    TransferMemoryHint, "transfer memory hint", ThreadTickCountDeprecated,
    "thread tick count deprecated")

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::SystemInfoType,
                       TotalPhysicalMemorySize, "total physical memory size",
                       UsedPhysicalMemorySize, "used physical memory size",
                       InitialProcessIdRange, "initial process id range")

ENABLE_ENUM_FORMATTING(hydra::horizon::kernel::ThreadActivity, None, "none",
                       Runnable, "runnable")
