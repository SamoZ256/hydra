#pragma once

#include <cstdint>

namespace hydra {

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using i128 = __int128_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u128 = __uint128_t;
using usize = size_t;
using uptr = uintptr_t;
using f32 = float;
using f64 = double;

using bool32 = u32;

using uuid_t = u128;
using magic4_t = u32;
using paddr_t = uptr;
using vaddr_t = uptr;
using gpu_vaddr_t = uptr;
using handle_id_t = u32;

constexpr handle_id_t INVALID_HANDLE_ID = 0;

} // namespace hydra
