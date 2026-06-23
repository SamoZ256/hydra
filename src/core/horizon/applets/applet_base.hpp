#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/am/internal/library_applet_controller.hpp"

namespace hydra::horizon::applets {

class AppletBase {
  public:
    AppletBase(services::am::internal::LibraryAppletController& controller_)
        : controller{controller_} {}
    virtual ~AppletBase() noexcept = default;

    void Start(System& system);

    result_t GetResult() const { return result; }

  protected:
    virtual result_t Run(System& system) = 0;

    // Helpers

    // Data
    io::MemoryStream PopInDataRaw() {
        auto data = controller.PopInData()->GetData();
        return {data};
    }

    template <typename T>
    T PopInData() {
        auto stream = PopInDataRaw();
        ASSERT(stream.GetSize() >= sizeof(T), Applets,
               "Not enough space ({} < {})", stream.GetSize(), sizeof(T));

        return stream.Read<T>();
    }

    void PushOutDataRaw(std::vector<u8> data) {
        controller.PushOutData(new services::am::IStorage(std::move(data)));
    }

    template <typename T>
    void PushOutData(const T& data) {
        std::vector<u8> bytes(sizeof(T));
        std::memcpy(bytes.data(), &data, sizeof(T));
        PushOutDataRaw(std::move(bytes));
    }

    // Interactive data
    io::MemoryStream PopInteractiveInDataRaw() {
        // TODO: wait
        // controller.GetInteractiveInDataEvent().Wait();
        auto data = controller.PopInteractiveInData()->GetData();
        return {data};
    }

    template <typename T>
    T PopInteractiveInData() {
        auto stream = PopInteractiveInDataRaw();
        ASSERT(stream.GetSize() >= sizeof(T), Applets,
               "Not enough space ({} < {})", stream.GetSize(), sizeof(T));

        return stream.Read<T>();
    }

    void PushInteractiveOutDataRaw(std::vector<u8> data) {
        controller.PushInteractiveOutData(
            new services::am::IStorage(std::move(data)));
    }

    template <typename T>
    void PushInteractiveOutData(const T& data) {
        std::vector<u8> bytes(sizeof(T));
        std::memcpy(bytes.data(), &data, sizeof(T));
        PushInteractiveOutDataRaw(std::move(bytes));
    }

  private:
    services::am::internal::LibraryAppletController& controller;

    std::optional<std::jthread> thread;
    result_t result{RESULT_SUCCESS};
};

} // namespace hydra::horizon::applets
