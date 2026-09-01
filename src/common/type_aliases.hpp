#pragma once

#include <ztd/ztd.hpp>

namespace hydra {

using i8 = ztd::i8;
using i16 = ztd::i16;
using i32 = ztd::i32;
using i64 = ztd::i64;
using i128 = ztd::i128;
using u8 = ztd::u8;
using u16 = ztd::u16;
using u32 = ztd::u32;
using u64 = ztd::u64;
using u128 = ztd::u128;
using usize = ztd::usize;
using uptr = ztd::uptr;
using f32 = ztd::f32;
using f64 = ztd::f64;

using bool32 = u32;

using uuid_t = u128;
using magic4_t = u32;
using paddr_t = u64;
using vaddr_t = u64;
using gpu_vaddr_t = u64;

using namespace ztd::mem::literals;

} // namespace hydra
