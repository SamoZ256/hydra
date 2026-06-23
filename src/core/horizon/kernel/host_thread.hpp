#pragma once

#include "core/horizon/kernel/thread.hpp"

namespace hydra::horizon::kernel {

using should_stop_fn_t = std::function<bool()>;
using run_callback_fn_t = std::function<void(const should_stop_fn_t&)>;

class HostThread : public IThread {
  public:
    HostThread(Process* process, i32 priority, run_callback_fn_t run_callback_,
               std::string_view debug_name = "Thread")
        : IThread(process, priority, debug_name),
          run_callback{std::move(run_callback_)} {}
    ~HostThread() override { delete[] tls; }

    uptr GetTlsPtr() const override { return reinterpret_cast<uptr>(tls); }

  protected:
    void Run() override;

  private:
    run_callback_fn_t run_callback;

    u8* tls = new u8[TLS_SIZE]; // TODO: stack allocate
};

} // namespace hydra::horizon::kernel
