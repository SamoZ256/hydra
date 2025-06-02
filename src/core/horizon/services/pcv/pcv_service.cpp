#include "core/horizon/services/pcv/pcv_service.hpp"

ENABLE_ENUM_FORMATTING(
    hydra::horizon::services::pcv::ModuleId, Cpu, "CPU", Gpu, "GPU", I2s1,
    "I2S1", I2s2, "I2S2", I2s3, "I2S3", Pwm, "PWM", I2c1, "I2C1", I2c2, "I2C2",
    I2c3, "I2C3", I2c4, "I2C4", I2c5, "I2C5", I2c6, "I2C6", Spi1, "SPI1", Spi2,
    "SPI2", Spi3, "SPI3", Spi4, "SPI4", Disp1, "DISP1", Disp2, "DISP2", Isp,
    "ISP", Vi, "VI", Sdmmc1, "SDMMC1", Sdmmc2, "SDMMC2", Sdmmc3, "SDMMC3",
    Sdmmc4, "SDMMC4", Owr, "OWR", Csite, "CSITE", Tsec, "TSEC", Mselect,
    "MSELECT", Hda2codec2x, "HDA2CODEC2X", Actmon, "ACTMON", I2cSlow, "I2CSLOW",
    Sor1, "SOR1", Sata, "SATA", Hda, "HDA", XusbCoreHostSrc,
    "XUSB_CORE_HOST_SRC", XusbFalconSrc, "XUSB_FALCON_SRC", XusbFsSrc,
    "XUSB_FS_SRC", XusbCoreDevSrc, "XUSB_CORE_DEV_SRC", XusbSsSrc,
    "XUSB_SS_SRC", UartA, "UARTA", UartB, "UARTB", UartC, "UARTC", UartD,
    "UARTD", Host1x, "HOST1X", Entropy, "ENTROPY", SocTherm, "SOC_THERM", Vic,
    "VIC", Nvenc, "NVENC", Nvjpg, "NVJPG", Nvdec, "NVDEC", Qspi, "QSPI", ViI2c,
    "VI_I2C", Tsecb, "TSECB", Ape, "APE", AudioDsp, "AUDIO_DSP", AudioUart,
    "AUDIO_UART", Emc, "EMC", Plle, "PLLE", PlleHwSeq, "PLLE_HW_SEQ", Dsi,
    "DSI", Maud, "MAUD", Dpaux1, "DPAUX1", MipiCal, "MIPI_CAL", UartFstMipiCal,
    "UART_FST_MIPI_CAL", Osc, "OSC", SysBus, "SYS_BUS", SorSafe, "SOR_SAFE",
    XusbSs, "XUSB_SS", XusbHost, "XUSB_HOST", XusbDevice, "XUSB_DEVICE",
    Extperiph1, "EXTPERIPH1", Ahub, "AHUB", Hda2hdmicodec, "HDA2HDMICODEC",
    Gpuaux, "GPUAUX", UsbD, "USBD", Usb2, "USB2", Pcie, "PCIE", Afi, "AFI",
    PciExClk, "PCI_EX_CLK", PExUsbPhy, "PEX_USB_PHY", XUsbPadCtl,
    "XUSB_PAD_CTL", Apbdma, "APBDMA", Usb2TrkClk, "USB2_TRK_CLK", XUsbIoPll,
    "XUSB_IO_PLL", XUsbIoPllHwSeq, "XUSB_IO_PLL_HW_SEQ", Cec, "CEC", Extperiph2,
    "EXTPERIPH2", OscClk, "OSC_CLK")

namespace hydra::horizon::services::pcv {

DEFINE_SERVICE_COMMAND_TABLE(IPcvService, 2, SetClockRate, 3, GetClockRate)

result_t IPcvService::SetClockRate(ModuleId module_id, u32 rate) {
    if (module_id >= ModuleId::Count) {
        LOG_ERROR(Services, "Invalid module ID {}", (u32)module_id);
        return MAKE_RESULT(Svc,
                           kernel::Error::InvalidEnumValue); // TODO: module
    }

    clock_rates[(u32)module_id] = rate;
    return RESULT_SUCCESS;
}

result_t IPcvService::GetClockRate(ModuleId module_id, u32* out_rate) {
    if (module_id >= ModuleId::Count) {
        LOG_ERROR(Services, "Invalid module ID {}", (u32)module_id);
        return MAKE_RESULT(Svc,
                           kernel::Error::InvalidEnumValue); // TODO: module
    }

    *out_rate = clock_rates[(u32)module_id];
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::pcv
