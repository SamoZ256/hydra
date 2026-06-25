#include "ztd/compress/lz4.hpp"

#include <cstring>

namespace ztd::compress {

namespace {

auto getLength(std::span<const u8> src, u32& cmp_pos, u32 length) noexcept
    -> u32 {
    u8 sum = 0;
    if (length == 0xf) {
        do {
            length += sum = src[cmp_pos++];
        } while (sum == 0xff);
    }

    return length;
}

} // namespace

auto decompressLz4(std::span<const u8> src, std::span<u8> dst) noexcept
    -> void {
    u32 cmp_pos = 0;
    u32 dec_pos = 0;

    do {
        u8 token = src[cmp_pos++];

        u32 enc_count = (token >> 0) & 0xf;
        u32 lit_count = (token >> 4) & 0xf;

        // Copy literal chunk
        lit_count = getLength(src, cmp_pos, lit_count);

        std::memcpy(dst.data() + dec_pos, src.data() + cmp_pos, lit_count);

        cmp_pos += lit_count;
        dec_pos += lit_count;

        if (cmp_pos >= src.size()) {
            break;
        }

        // Copy compressed chunk
        u32 back = static_cast<u32>(src[cmp_pos++]) << 0u;
        back |= static_cast<u32>(src[cmp_pos++]) << 8u;

        enc_count = getLength(src, cmp_pos, enc_count) + 4;

        u32 enc_pos = dec_pos - back;

        if (enc_count <= back) {
            std::memcpy(dst.data() + dec_pos, dst.data() + enc_pos, enc_count);

            dec_pos += enc_count;
        } else {
            while (enc_count-- > 0) {
                dst[dec_pos++] = dst[enc_pos++];
            }
        }
    } while (cmp_pos < src.size() && dec_pos < dst.size());
}

} // namespace ztd::compress
