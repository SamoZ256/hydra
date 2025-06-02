#pragma once

#include "core/horizon/const.hpp"

namespace hydra::horizon::services::nvdrv {

enum class NvResult : u32 {
    Success,
    NotImplemented,
    NotSupported,
    NotInitialized,
    BadParameter,
    Timeout,
    InsufficientMemory,
    ReadOnlyAttribute,
    InvalidState,
    InvalidAddress,
    InvalidSize,
    BadValue,
    AlreadyAllocated,
    Busy,
    ResourceError,
    CountMismatch,
    SharedMemoryTooSmall = 0x1000,
    FileOperationFailed = 0x30003,
    DirectoryOperationFailed = 0x30004,
    NotAvailableInProduction = 0x30006,
    IoctlFailed = 0x3000F,
    AccessDenied = 0x30010,
    FileNotFound = 0x30013,
    ModuleNotPresent = 0xA000E,
};

enum class NvInternalResult : i32 {
    Success = 0,
    OperationNotPermitted = -1,
    NoEntry = -2,
    Interrupted = -4,
    IoError = -5,
    DeviceNotFound = -6,
    BadFileNumber = -9,
    TryAgain = -11,
    OutOfMemory = -12,
    AccessDenied = -13,
    BadAddress = -14,
    Busy = -16,
    NotADirectory = -20,
    InvalidInput = -22,
    FileTableOverflow = -23,
    Unknown0x18 = -24,
    NotSupported = -25,
    FileTooBig = -27,
    NoSpaceLeft = -28,
    ReadOnlyAttribute = -30,
    NotImplemented = -38,
    InvalidState = -40,
    Restart = -85,
    InvalidAddress = -99,
    TimedOut = -110,
    Unknown0x72 = -114,
};

} // namespace hydra::horizon::services::nvdrv
