#pragma once

#include <cstdint>
#include <cstdlib>

namespace hydra {

using i8 = std::int8_t;
using i16 = std::int16_t;
using i32 = std::int32_t;
using i64 = std::int64_t;
using i128 = __int128_t;
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using u128 = __uint128_t;
using usize = std::size_t;
using uptr = std::uintptr_t;
using f32 = float;
using f64 = double;

using bool32 = u32;

using uuid_t = u128;
using magic4_t = u32;
using paddr_t = u64;
using vaddr_t = u64;
using gpu_vaddr_t = u64;
using handle_id_t = u32;

constexpr handle_id_t INVALID_HANDLE_ID = 0;

} // namespace hydra
