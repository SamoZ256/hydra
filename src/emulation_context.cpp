#include "emulation_context.hpp"
#include "horizon/loader/loader_base.hpp"
#include "horizon/loader/nro_loader.hpp"

// HACK
void SET_INSTRUCTION(u32* data, i64 addr, u32 new_instruction) {
    data[addr / 4] = new_instruction;
}

// HACK
#define NOP 0xd503201fu
#define RET 0xd65f03c0u
#define MOV_X0_XZR 0xaa1f03e0u
#define BRK 0xd4200000u

namespace Hydra {

EmulationContext::EmulationContext() {

    // Emulation
    // TODO: choose based on CPU backend
    {
        cpu = new Hydra::HW::TegraX1::CPU::Hypervisor::CPU();
    }

    gpu = new Hydra::HW::TegraX1::GPU::GPU(cpu->GetMMU());

    builtin_display = new Hydra::HW::Display::Display();

    bus = new Hydra::HW::Bus();
    bus->ConnectDisplay(builtin_display, 0);

    os = new Hydra::Horizon::OS(*bus, cpu->GetMMU());
}

EmulationContext::~EmulationContext() {
    for (auto t : threads) {
        // Force the thead to exit
        delete t;
    }
}

void EmulationContext::Start(const std::string& rom_filename) {
    // HACK
    // u32* data = (u32*)(rom->GetRom().data());
    // SET_INSTRUCTION(data, 0x00000c28,
    //                MOV_X0_XZR); // appletInitialize, infinite sleep on exit
    // SET_INSTRUCTION(data, 0x0000161c,
    //                NOP); // _fsdevUnmountDeviceStruct, crashes due to "str
    //                x5,

    // HACK: for testing
    // SET_INSTRUCTION(data, 0xf1e8, BRK);

    // Load ROM
    // TODO: choose the loader based on the file format
    Horizon::Loader::LoaderBase* loader;
    {
        loader = new Horizon::Loader::NROLoader();
    }
    loader->LoadROM(rom_filename);

    // Main thread
    std::thread* t = new std::thread(
        [&](std::string rom_filename) {
            // Main thread
            Hydra::HW::TegraX1::CPU::ThreadBase* main_thread =
                cpu->CreateThread();
            os->GetKernel().ConfigureMainThread(main_thread, rom_filename);

            // Run
            main_thread->Run();

            // Cleanup
            delete main_thread;

            // Notify that emulation has ended
            is_running = false;
        },
        rom_filename);

    is_running = true;
}

} // namespace Hydra
