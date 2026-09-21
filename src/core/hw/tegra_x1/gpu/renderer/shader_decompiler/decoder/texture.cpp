#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/decoder/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder {

namespace {

// TODO: nodep
void emitTextureQuery(DecoderContext& context, pred_t pred, bool pred_inv,
                      TextureQuery query, reg_t dst, u8 write_mask, reg_t src,
                      u32 cbuf_index) {
    // TODO: src
    (void)src;

    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    // NOLINTNEXTLINE(readability-trivial-switch)
    switch (query) {
    case TextureQuery::Dimensions:
        for (u32 i = 0, mask = write_mask; mask != 0x0; i++, mask >>= 1) {
            if ((mask & 1) != 0u) {
                const auto res =
                    context.builder.opTextureQueryDimension(cbuf_index, i);
                context.builder.opCopy(
                    ir::Value::createRegister(dst + static_cast<u8>(i)), res);
            }
        }
        break;
    default:
        LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Texture query {}", query);
        break;
    }

    if (conditional)
        context.builder.opEndIf();
}

u8 toTexComponent(TextureComponent component) {
    switch (component) {
    case TextureComponent::R:
        return 0;
    case TextureComponent::G:
        return 1;
    case TextureComponent::B:
        return 2;
    case TextureComponent::A:
        return 3;
    }
}

void copyTextureResult(ir::Builder& builder, std::array<reg_t, 4> dsts,
                       ir::Value res, u8 component_mask) {
    u32 output_index = 0;
    for (u8 i = 0; i < 4; i++) {
        if ((component_mask & (1 << i)) == 0x0)
            continue;

        builder.opCopy(ir::Value::createRegister(dsts[output_index++],
                                                 ir::ScalarType::F32),
                       builder.opVectorExtract(res, i));
    }
}

void copyTextureResult(ir::Builder& builder, reg_t dst, ir::Value res,
                       u8 component_mask) {
    copyTextureResult(builder, {dst + 0, dst + 1, dst + 2, dst + 3}, res,
                      component_mask);
}

constexpr std::array<std::array<u8, 8>, 2> COMPONENT_MASK_LUT = {{
    {0b0001, 0b0010, 0b0100, 0b1000, 0b0011, 0b1001, 0b1010, 0b1100},
    {0b0111, 0b1011, 0b1101, 0b1110, 0b1111, 0b0000, 0b0000, 0b0000},
}};

// TODO: nodep
void emitTextureSample(DecoderContext& context, pred_t pred, bool pred_inv,
                       bool int_coords, TextureSampleTarget target, reg_t dst0,
                       reg_t dst1, u8 write_mask, reg_t src_a, reg_t src_b,
                       u32 cbuf_index) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

#define RA() ir::Value::createRegister(src_a++, ir::ScalarType::F32)
#define RB() ir::Value::createRegister(src_b++, ir::ScalarType::F32)

    TextureType type;
    TextureSampleFlags flags =
        (int_coords ? TextureSampleFlags::IntCoords : TextureSampleFlags::None);
    ir::Value array_index = ir::Value::createUndefined();
    std::vector<ir::Value> coords;
    ir::Value cmp_value = ir::Value::createUndefined();
    ir::Value lod = ir::Value::createUndefined();
    switch (target) {
    case TextureSampleTarget::_1DLodZero:
        type = TextureType::_1D;
        flags = TextureSampleFlags::Lod;
        coords.push_back(RA());
        lod = ir::Value::createConstantF(0.0f);
        break;
    case TextureSampleTarget::_2D:
        type = TextureType::_2D;
        coords.push_back(RA());
        coords.push_back(RB());
        break;
    case TextureSampleTarget::_2DLodZero:
        type = TextureType::_2D;
        flags = TextureSampleFlags::Lod;
        coords.push_back(RA());
        coords.push_back(RB());
        lod = ir::Value::createConstantF(0.0f);
        break;
    case TextureSampleTarget::_2DLodLevel:
        type = TextureType::_2D;
        flags = TextureSampleFlags::Lod;
        coords.push_back(RA());
        coords.push_back(RA());
        lod = RB();
        break;
    case TextureSampleTarget::_2DDepthCompare:
        type = TextureType::_2D;
        flags = TextureSampleFlags::DepthCompare;
        coords.push_back(RA());
        coords.push_back(RA());
        cmp_value = RB();
        break;
    case TextureSampleTarget::_2DLodLevelDepthCompare:
        type = TextureType::_2D;
        flags = TextureSampleFlags::DepthCompare | TextureSampleFlags::Lod;
        coords.push_back(RA());
        coords.push_back(RA());
        cmp_value = RB();
        lod = RB();
        break;
    case TextureSampleTarget::_2DLodZeroDepthCompare:
        type = TextureType::_2D;
        flags = TextureSampleFlags::DepthCompare | TextureSampleFlags::Lod;
        coords.push_back(RA());
        coords.push_back(RA());
        cmp_value = RB();
        lod = ir::Value::createConstantF(0.0f);
        break;
    case TextureSampleTarget::_2DArray:
        type = TextureType::_2DArray;
        array_index = RA();
        coords.push_back(RA());
        coords.push_back(RB());
        break;
    case TextureSampleTarget::_2DArrayLodZero:
        type = TextureType::_2DArray;
        flags = TextureSampleFlags::Lod;
        array_index = RA();
        coords.push_back(RA());
        coords.push_back(RB());
        lod = ir::Value::createConstantF(0.0f);
        break;
    case TextureSampleTarget::_2DArrayLodZeroDepthCompare:
        type = TextureType::_2DArray;
        flags = TextureSampleFlags::DepthCompare | TextureSampleFlags::Lod;
        array_index = RA();
        coords.push_back(RA());
        coords.push_back(RB());
        cmp_value = RB();
        lod = ir::Value::createConstantF(0.0f);
        break;
    case TextureSampleTarget::_3D:
        type = TextureType::_3D;
        coords.push_back(RA());
        coords.push_back(RA());
        coords.push_back(RB());
        break;
    case TextureSampleTarget::_3DLodZero:
        type = TextureType::_3D;
        flags = TextureSampleFlags::Lod;
        coords.push_back(RA());
        coords.push_back(RA());
        coords.push_back(RB());
        lod = ir::Value::createConstantF(0.0f);
        break;
    case TextureSampleTarget::Cube:
        type = TextureType::Cube;
        coords.push_back(RA());
        coords.push_back(RA());
        coords.push_back(RB());
        break;
    case TextureSampleTarget::CubeLodLevel:
        type = TextureType::Cube;
        flags = TextureSampleFlags::Lod;
        coords.push_back(RA());
        coords.push_back(RA());
        coords.push_back(RB());
        lod = RB();
        break;
    }

    const auto coords_v =
        context.builder.opVectorConstruct(ir::ScalarType::F32, coords);
    ir::Value res = context.builder.opTextureSample(
        cbuf_index, type, flags, array_index, coords_v, cmp_value, lod);

    const auto component_mask =
        COMPONENT_MASK_LUT[dst1 == RZ ? 0 : 1][write_mask];
    ASSERT_DEBUG(component_mask != 0, ShaderDecompiler,
                 "Invalid component mask");
    copyTextureResult(context.builder, {dst0 + 0, dst0 + 1, dst1 + 0, dst1 + 1},
                      res, component_mask);

    if (conditional)
        context.builder.opEndIf();
}

// TODO: ndv, nodep, dc, dst_pred, aoffi, lod, lc
void emitTextureSample2(DecoderContext& context, pred_t pred, bool pred_inv,
                        bool int_coords, TextureDimension dim, reg_t dst,
                        u8 component_mask, reg_t src_a, reg_t src_b,
                        u32 cbuf_index) {
    (void)src_b;

    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

#define RA() ir::Value::createRegister(src_a++, ir::ScalarType::F32)
#define RB() ir::Value::createRegister(src_b++, ir::ScalarType::F32)

    TextureType type;
    TextureSampleFlags flags =
        (int_coords ? TextureSampleFlags::IntCoords : TextureSampleFlags::None);
    ir::Value array_index = ir::Value::createUndefined();
    std::vector<ir::Value> coords;
    switch (dim) {
    case TextureDimension::_1D:
        type = TextureType::_1D;
        coords.push_back(RA());
        break;
    case TextureDimension::_1DArray:
        type = TextureType::_1DArray;
        array_index = RA();
        coords.push_back(RA());
        break;
    case TextureDimension::_2D:
        type = TextureType::_2D;
        coords.push_back(RA());
        coords.push_back(RA());
        break;
    case TextureDimension::_2DArray:
        type = TextureType::_2DArray;
        array_index = RA();
        coords.push_back(RA());
        coords.push_back(RA());
        break;
    case TextureDimension::_3D:
        type = TextureType::_3D;
        coords.push_back(RA());
        coords.push_back(RA());
        coords.push_back(RA());
        break;
    case TextureDimension::_3DArray:
        type = TextureType::_3DArray;
        array_index = RA();
        coords.push_back(RA());
        coords.push_back(RA());
        coords.push_back(RA());
        break;
    case TextureDimension::Cube:
        type = TextureType::Cube;
        coords.push_back(RA());
        coords.push_back(RA());
        coords.push_back(RA());
        break;
    case TextureDimension::CubeArray:
        type = TextureType::CubeArray;
        array_index = RA();
        coords.push_back(RA());
        coords.push_back(RA());
        coords.push_back(RA());
        break;
    }

    const auto coords_v =
        context.builder.opVectorConstruct(ir::ScalarType::F32, coords);
    ir::Value res = context.builder.opTextureSample(
        cbuf_index, type, flags, array_index, coords_v,
        ir::Value::createUndefined(), ir::Value::createUndefined());
    copyTextureResult(context.builder, dst, res, component_mask);

    if (conditional)
        context.builder.opEndIf();
}

// TODO: dim, ndv, nodep, dc, offset, lc, dst_pred
void emitTextureGather(DecoderContext& context, pred_t pred, bool pred_inv,
                       TextureComponent component, reg_t dst, u8 component_mask,
                       reg_t src_a, reg_t src_b, u32 cbuf_index) {
    // TODO: src B
    (void)src_b;

    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    const auto coords_v = context.builder.opVectorConstruct(
        ir::ScalarType::F32,
        {ir::Value::createRegister(src_a + 0, ir::ScalarType::F32),
         ir::Value::createRegister(src_a + 1, ir::ScalarType::F32)});
    const auto res = context.builder.opTextureGather(cbuf_index, coords_v,
                                                     toTexComponent(component));
    copyTextureResult(context.builder, dst, res, component_mask);

    if (conditional)
        context.builder.opEndIf();
}

// TODO: nodep, dc, aoffi
void emitTextureGather2(DecoderContext& context, pred_t pred, bool pred_inv,
                        TextureComponent component, reg_t dst0, reg_t dst1,
                        reg_t src_a, reg_t src_b, u32 cbuf_index) {
    const auto conditional = handlePredCond(context.builder, pred, pred_inv);

    const auto coords_v = context.builder.opVectorConstruct(
        ir::ScalarType::F32,
        {ir::Value::createRegister(src_a, ir::ScalarType::F32),
         ir::Value::createRegister(src_b, ir::ScalarType::F32)});
    const auto res = context.builder.opTextureGather(cbuf_index, coords_v,
                                                     toTexComponent(component));
    copyTextureResult(context.builder, {dst0 + 0, dst0 + 1, dst1 + 0, dst1 + 1},
                      res, 0b1111);

    if (conditional)
        context.builder.opEndIf();
}

} // namespace

void emitTxq(DecoderContext& context, InstTxq inst) {
    emitTextureQuery(context, inst.pred, inst.pred_inv, inst.query, inst.dst,
                     inst.write_mask, inst.src, inst.cbuf_index);
}

void emitTexs(DecoderContext& context, InstTexs inst) {
    emitTextureSample(context, inst.pred, inst.pred_inv, false, inst.target,
                      inst.dst0, inst.dst1, inst.write_mask, inst.src_a,
                      inst.src_b, inst.cbuf_index);
}

void emitTlds(DecoderContext& context, InstTlds inst) {
    emitTextureSample(context, inst.pred, inst.pred_inv, true, inst.target,
                      inst.dst0, inst.dst1, inst.write_mask, inst.src_a,
                      inst.src_b, inst.cbuf_index);
}

void emitTex(DecoderContext& context, InstTex inst) {
    emitTextureSample2(context, inst.pred, inst.pred_inv, false, inst.dim,
                       inst.dst, inst.write_mask, inst.src_a, inst.src_b,
                       inst.cbuf_index);
}

void emitTld4(DecoderContext& context, InstTld4 inst) {
    // TODO: why does this mess up SMO title screen?
    emitTextureGather(context, inst.pred, inst.pred_inv, inst.component,
                      inst.dst, inst.write_mask, inst.src_a, inst.src_b,
                      inst.cbuf_index);
}

void emitTld4s(DecoderContext& context, InstTld4s inst) {
    emitTextureGather2(context, inst.pred, inst.pred_inv, inst.component,
                       inst.dst0, inst.dst1, inst.src_a, inst.src_b,
                       inst.cbuf_index);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::decoder
