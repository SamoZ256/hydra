#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/am/internal/library_applet_controller.hpp"

namespace hydra::horizon::applets {

class AppletBase {
  public:
    explicit AppletBase(
        services::am::internal::LibraryAppletController& controller_)
        : controller{controller_} {}
    virtual ~AppletBase() noexcept = default;

    void start(System& system);

    result_t getResult() const { return result; }

  protected:
    virtual result_t run(System& system) = 0;

    // Helpers

    // Data
    ztd::io::MemoryStream popInDataRaw() {
        auto data = controller.popInData()->getData();
        return ztd::io::MemoryStream{data};
    }

    template <typename T>
    T popInData() {
        auto stream = popInDataRaw();
        ASSERT(stream.getSize() >= sizeof(T), Applets,
               "Not enough space ({} < {})", stream.getSize(), sizeof(T));

        return stream.read<T>();
    }

    void pushOutDataRaw(std::vector<u8> data) {
        controller.pushOutData(new services::am::IStorage(std::move(data)));
    }

    template <typename T>
    void pushOutData(const T& data) {
        std::vector<u8> bytes(sizeof(T));
        std::memcpy(bytes.data(), &data, sizeof(T));
        pushOutDataRaw(std::move(bytes));
    }

    // Interactive data
    ztd::io::MemoryStream popInteractiveInDataRaw() {
        // TODO: wait
        // controller.GetInteractiveInDataEvent().Wait();
        auto data = controller.popInteractiveInData()->getData();
        return ztd::io::MemoryStream{data};
    }

    template <typename T>
    T popInteractiveInData() {
        auto stream = popInteractiveInDataRaw();
        ASSERT(stream.getSize() >= sizeof(T), Applets,
               "Not enough space ({} < {})", stream.getSize(), sizeof(T));

        return stream.read<T>();
    }

    void pushInteractiveOutDataRaw(std::vector<u8> data) {
        controller.pushInteractiveOutData(
            new services::am::IStorage(std::move(data)));
    }

    template <typename T>
    void pushInteractiveOutData(const T& data) {
        std::vector<u8> bytes(sizeof(T));
        std::memcpy(bytes.data(), &data, sizeof(T));
        pushInteractiveOutDataRaw(std::move(bytes));
    }

  private:
    services::am::internal::LibraryAppletController& controller;

    std::optional<std::jthread> thread;
    result_t result{RESULT_SUCCESS};
};

} // namespace hydra::horizon::applets
