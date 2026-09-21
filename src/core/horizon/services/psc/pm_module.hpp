#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::kernel {
class Event;
}

namespace hydra::horizon::services::psc {

enum class PmModuleId : u32 {
    Reserved0 = 0,
    Reserved1 = 1,
    Reserved2 = 2,
    Socket = 3,
    Usb = 4,
    Ethernet = 5,
    Fgm = 6,
    PcvClock = 7,
    PcvVoltage = 8,
    Gpio = 9,
    Pinmux = 10,
    Uart = 11,
    I2c = 12,
    I2cPcv = 13,
    Spi = 14,
    Pwm = 15,
    Psm = 16,
    Tc = 17,
    Omm = 18,
    Pcie = 19,
    Lbl = 20,
    Display = 21,
    Hid = 24,
    SocketWlan = 25,
    Wlan = 26,
    Fs = 27,
    Audio = 28,
    Tm = 29,
    TmHostIo = 30,
    Bluetooth = 31,
    Bpc = 32,
    Fan = 33,
    Pcm = 34,
    Nfc = 35,
    Apm = 36,
    Btm = 37,
    Nifm = 38,
    GpioLow = 39,
    Npns = 40,
    Lm = 41,
    Bcat = 42,
    Time = 43,
    Pctl = 44,
    Erpt = 45,
    Eupld = 46,
    Friends = 47,
    Bgtc = 48,
    Account = 49,
    Sasbus = 50,
    Ntc = 51,
    Idle = 52,
    Tcap = 53,
    PsmLow = 54,
    Ndd = 55,
    Olsc = 56,
    Rtc = 57,
    Regulator = 58,
    Clkrst = 59,
    Led = 60,
    NuiShell = 61,
    Powctl = 62,
    NotificationPresenter = 63,
    Notification = 64,
    Dhcpc = 65,
    Mnpp = 66,
    Sprofile = 67,
    NvHost = 101,
    NvDbgSvc = 105,
    Tspm = 106,
    Spsm = 127,
};

class IPmModule : public IService {
  public:
    IPmModule();

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* event;

    // Commands
    result_t initialize(kernel::Process* process, PmModuleId module_id,
                        InBuffer<BufferAttr::MapAlias> in_dep_buffer,
                        OutHandle<HandleAttr::Copy> out_event_handle);
};

} // namespace hydra::horizon::services::psc

ENABLE_ENUM_FORMATTING(
    hydra::horizon::services::psc::PmModuleId, Reserved0, "reserved 0",
    Reserved1, "reserved 1", Reserved2, "reserved 2", Socket, "socket", Usb,
    "usb", Ethernet, "ethernet", Fgm, "fgm", PcvClock, "pcv clock", PcvVoltage,
    "pcv voltage", Gpio, "gpio", Pinmux, "pinmux", Uart, "uart", I2c, "i2c",
    I2cPcv, "i2c pcv", Spi, "spi", Pwm, "pwm", Psm, "psm", Tc, "tc", Omm, "omm",
    Pcie, "pcie", Lbl, "lbl", Display, "display", Hid, "hid", SocketWlan,
    "socket wlan", Wlan, "wlan", Fs, "fs", Audio, "audio", Tm, "tm", TmHostIo,
    "tm host io", Bluetooth, "bluetooth", Bpc, "bpc", Fan, "fan", Pcm, "pcm",
    Nfc, "nfc", Apm, "apm", Btm, "btm", Nifm, "nifm", GpioLow, "gpio low", Npns,
    "npns", Lm, "lm", Bcat, "bcat", Time, "time", Pctl, "pctl", Erpt, "erpt",
    Eupld, "eupld", Friends, "friends", Bgtc, "bgtc", Account, "account",
    Sasbus, "sasbus", Ntc, "ntc", Idle, "idle", Tcap, "tcap", PsmLow, "psm low",
    Ndd, "ndd", Olsc, "olsc", Rtc, "rtc", Regulator, "regulator", Clkrst,
    "clkrst", Led, "led", NuiShell, "nui shell", Powctl, "powctl",
    NotificationPresenter, "notification presenter", Notification,
    "notification", Dhcpc, "dhcpc", Mnpp, "mnpp", Sprofile, "sprofile", NvHost,
    "nv host", NvDbgSvc, "nv dbg svc", Tspm, "tspm", Spsm, "spsm")
