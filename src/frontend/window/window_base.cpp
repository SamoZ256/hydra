#include "frontend/window/sdl3/window.hpp"

namespace Hydra::Frontend::Window {

void WindowBase::InitializeEmulationContext(const std::string& rom_filename) {
    emulation_context = new EmulationContext();

    void* surface = CreateSurfaceImpl();
    emulation_context->GetGPU()->GetRenderer()->SetSurface(surface);

    emulation_context->Start(rom_filename);
}

void WindowBase::Run() {}

void WindowBase::Present() {
    // TODO: correct?
    // Inform the app that we want to display the window
    Horizon::StateManager::GetInstance().SendMessage(
        Horizon::AppletMessage::RequestToDisplay);

    // TODO: don't hardcode the display id
    auto display = emulation_context->GetBus()->GetDisplay(0);
    auto layer = display->GetPresentableLayer();
    if (!layer)
        return;

    u32 binder_id = layer->GetBinderId();
    auto& binder =
        emulation_context->GetOS()->GetDisplayBinderManager().GetBinder(
            binder_id);
    i32 slot = binder.ConsumeBuffer();
    if (slot == -1)
        return;
    const auto& buffer = binder.GetBuffer(slot);

    auto gpu = emulation_context->GetGPU();
    auto texture = gpu->GetTexture(buffer);
    gpu->GetRenderer()->Present(texture);
}

} // namespace Hydra::Frontend::Window
