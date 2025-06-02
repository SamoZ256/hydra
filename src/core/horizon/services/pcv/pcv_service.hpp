#pragma once

#include "core/horizon/services/const.hpp"
#include "core/hw/tegra_x1/cpu/const.hpp"

namespace hydra::horizon::services::pcv {

enum class ModuleId : u32 {
    Cpu = 0,
    Gpu = 1,
    I2s1 = 2,
    I2s2 = 3,
    I2s3 = 4,
    Pwm = 5,
    I2c1 = 6,
    I2c2 = 7,
    I2c3 = 8,
    I2c4 = 9,
    I2c5 = 10,
    I2c6 = 11,
    Spi1 = 12,
    Spi2 = 13,
    Spi3 = 14,
    Spi4 = 15,
    Disp1 = 16,
    Disp2 = 17,
    Isp = 18,
    Vi = 19,
    Sdmmc1 = 20,
    Sdmmc2 = 21,
    Sdmmc3 = 22,
    Sdmmc4 = 23,
    Owr = 24,
    Csite = 25,
    Tsec = 26,
    Mselect = 27,
    Hda2codec2x = 28,
    Actmon = 29,
    I2cSlow = 30,
    Sor1 = 31,
    Sata = 32,
    Hda = 33,
    XusbCoreHostSrc = 34,
    XusbFalconSrc = 35,
    XusbFsSrc = 36,
    XusbCoreDevSrc = 37,
    XusbSsSrc = 38,
    UartA = 39,
    UartB = 40,
    UartC = 41,
    UartD = 42,
    Host1x = 43,
    Entropy = 44,
    SocTherm = 45,
    Vic = 46,
    Nvenc = 47,
    Nvjpg = 48,
    Nvdec = 49,
    Qspi = 50,
    ViI2c = 51,
    Tsecb = 52,
    Ape = 53,
    AudioDsp = 54,
    AudioUart = 55,
    Emc = 56,
    Plle = 57,
    PlleHwSeq = 58,
    Dsi = 59,
    Maud = 60,
    Dpaux1 = 61,
    MipiCal = 62,
    UartFstMipiCal = 63,
    Osc = 64,
    SysBus = 65,
    SorSafe = 66,
    XusbSs = 67,
    XusbHost = 68,
    XusbDevice = 69,
    Extperiph1 = 70,
    Ahub = 71,
    Hda2hdmicodec = 72,
    Gpuaux = 73,
    UsbD = 74,
    Usb2 = 75,
    Pcie = 76,
    Afi = 77,
    PciExClk = 78,
    PExUsbPhy = 79,
    XUsbPadCtl = 80,
    Apbdma = 81,
    Usb2TrkClk = 82,
    XUsbIoPll = 83,
    XUsbIoPllHwSeq = 84,
    Cec = 85,
    Extperiph2 = 86,
    OscClk = 87, // TODO: correct?

    Count = 88,
};

class IPcvService : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // TODO: other clock rates
    u32 clock_rates[(u32)ModuleId::Count] = {hw::tegra_x1::cpu::CLOCK_RATE_HZ,
                                             0};

    // Commands
    result_t SetClockRate(ModuleId module_id, u32 rate);
    result_t GetClockRate(ModuleId module_id, u32* out_rate);
};

} // namespace hydra::horizon::services::pcv
