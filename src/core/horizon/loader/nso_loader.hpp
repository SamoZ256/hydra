#pragma once

#include "core/horizon/loader/loader_base.hpp"

namespace hydra::horizon::loader {

struct Segment {
    u32 file_offset;
    u32 memory_offset;
    u32 size;
};

class NsoLoader : public LoaderBase {
  public:
    NsoLoader(filesystem::FileBase* file_,
              const std::string_view name_ = "main",
              const bool is_entry_point_ = true);

    void SetMainThreadParams(u8 priority, u8 core_number, u32 stack_size) {
        main_thread_priority = priority;
        main_thread_core_number = core_number;
        main_thread_stack_size = stack_size;
    }

    void LoadProcess(kernel::Process* process) override;

  private:
    filesystem::FileBase* file;
    std::string name;
    const bool is_entry_point;

    u32 text_offset;
    usize executable_size{0};
    struct {
        Segment seg;
        usize file_size;
        bool compressed;
    } segments[3];
    u32 dyn_str_offset;
    u32 dyn_str_size;
    u32 dyn_sym_offset;
    u32 dyn_sym_size;

    // TODO: what should the defaults be?
    u8 main_thread_priority{0x2c};
    u8 main_thread_core_number{0};
    u32 main_thread_stack_size{0x40000};
};

} // namespace hydra::horizon::loader
