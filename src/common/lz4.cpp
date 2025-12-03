#include "common/lz4.hpp"

namespace hydra {

namespace {

u32 get_length(const u8* src, u32& cmp_pos, u32 length) {
    u8 sum;
    if (length == 0xf) {
        do {
            length += sum = src[cmp_pos++];
        } while (sum == 0xff);
    }

    return length;
}

} // namespace

void decompress_lz4(const u8* src, usize src_size, u8* dst, usize dst_size) {
    u32 cmp_pos = 0;
    u32 dec_pos = 0;

    do {
        u8 token = src[cmp_pos++];

        u32 enc_count = (token >> 0) & 0xf;
        u32 lit_count = (token >> 4) & 0xf;

        // Copy literal chunk
        lit_count = get_length(src, cmp_pos, lit_count);

        memcpy(dst + dec_pos, src + cmp_pos, lit_count);

        cmp_pos += lit_count;
        dec_pos += lit_count;

        if (cmp_pos >= src_size) {
            break;
        }

        // Copy compressed chunk
        u32 back = src[cmp_pos++] << 0;
        back |= src[cmp_pos++] << 8;

        enc_count = get_length(src, cmp_pos, enc_count) + 4;

        u32 enc_pos = dec_pos - back;

        if (enc_count <= back) {
            memcpy(dst + dec_pos, dst + enc_pos, enc_count);

            dec_pos += enc_count;
        } else {
            while (enc_count-- > 0) {
                dst[dec_pos++] = dst[enc_pos++];
            }
        }
    } while (cmp_pos < src_size && dec_pos < dst_size);
}

} // namespace hydra
