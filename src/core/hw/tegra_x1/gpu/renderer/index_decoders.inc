DEFINE_DECODER_PROTOTYPE(quads_to_triangles) {
    for (u32 i = 0; i < count / 4; i++) {
        const u32 base = i * 4;
        const u32 base_index = i * 6;
        ADD_INDEX_AUTO(base + 0);
        ADD_INDEX_AUTO(base + 1);
        ADD_INDEX_AUTO(base + 2);
        ADD_INDEX_AUTO(base + 0);
        ADD_INDEX_AUTO(base + 2);
        ADD_INDEX_AUTO(base + 3);
    }
}

DEFINE_DECODER_PROTOTYPE(triangle_fan_to_triangle_strip) {
    for (u32 i = 0; i < count; i++) {
        u32 idx;
        GET_INDEX(idx, i);
        if (idx % 2 == 0)
            idx = idx / 2;
        else
            idx = count - 1 - idx / 2;
        ADD_INDEX(idx);
    }
}
