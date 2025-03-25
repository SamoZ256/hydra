#include "emulation_context.hpp"

#include "horizon/loader/nro_loader.hpp"
#include "horizon/loader/nso_loader.hpp"

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
    usize size;
    auto ifs = Hydra::open_file(rom_filename, size);

    std::string extension =
        rom_filename.substr(rom_filename.find_last_of(".") + 1);
    Horizon::Loader::LoaderBase* loader{nullptr};
    if (extension == "nro")
        loader = new Horizon::Loader::NROLoader();
    else if (extension == "nso")
        loader = new Horizon::Loader::NSOLoader(true);
    else
        LOG_ERROR(Other, "Unknown ROM extension \"{}\"", extension);

    FileReader reader(ifs, 0, size);
    loader->LoadROM(reader, rom_filename);
    delete loader;

    ifs.close();

    // Main thread
    std::thread* t = new std::thread([&]() {
        // Main thread
        Hydra::HW::TegraX1::CPU::ThreadBase* main_thread = cpu->CreateThread();
        os->GetKernel().ConfigureMainThread(main_thread);

        // Run
        main_thread->Run();

        // Cleanup
        delete main_thread;

        // Notify that emulation has ended
        is_running = false;
    });

    is_running = true;
}

} // namespace Hydra
