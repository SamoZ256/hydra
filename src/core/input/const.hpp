#pragma once

namespace hydra::input {

using code_t = u32;

template <typename Code> code_t make_code(u32 device_id, Code code) {
    return ((device_id & 0xff) << 24) | (static_cast<code_t>(code) & 0xffffff);
}

inline u32 get_code_device_id(code_t code) { return (code >> 24) & 0xff; }

template <typename Code> Code get_code_value(code_t code) {
    return static_cast<Code>(code & 0xffffff);
}

} // namespace hydra::input
