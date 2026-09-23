#pragma once

#include <ztd/ztd.hpp>

namespace hydra {

// clang-format off
using ztd::i8;
using ztd::i16;
using ztd::i32;
using ztd::i64;
using ztd::i128;
using ztd::u8;
using ztd::u16;
using ztd::u32;
using ztd::u64;
using ztd::u128;
using ztd::usize;
using ztd::uptr;
using ztd::f32;
using ztd::f64;
// clang-format on

using bool32 = u32;

using uuid_t = u128;
using magic4_t = u32;
using paddr_t = u64;
using vaddr_t = u64;
using gpu_vaddr_t = u64;

using namespace ztd::mem::literals;

} // namespace hydra
