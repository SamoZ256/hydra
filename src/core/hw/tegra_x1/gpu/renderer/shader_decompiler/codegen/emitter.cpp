#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/emitter.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/module.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen {

void Emitter::emit(const ir::Module& modul) {
    start();
    for (const auto& [name, func] : modul.getFunctions())
        emitFunction(func);
    finish();
}

void Emitter::emitInstruction(const ir::Instruction& inst) {
    switch (inst.getOpcode()) {
    // Data
    case ir::Opcode::Copy:
        emitCopy(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Cast:
        emitCast(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::BitCast:
        emitBitCast(inst.getDst(), inst.getOperand(0));
        break;

    // Arithmetic
    case ir::Opcode::Abs:
        emitAbs(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Neg:
        emitNeg(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Add:
        emitAdd(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::Multiply:
        emitMultiply(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::Fma:
        emitFma(inst.getDst(), inst.getOperand(0), inst.getOperand(1),
                inst.getOperand(2));
        break;
    case ir::Opcode::Min:
        emitMin(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::Max:
        emitMax(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::Clamp:
        emitClamp(inst.getDst(), inst.getOperand(0), inst.getOperand(1),
                  inst.getOperand(2));
        break;

    // Math
    case ir::Opcode::IsNan:
        emitIsNan(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Round:
        emitRound(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Floor:
        emitFloor(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Ceil:
        emitCeil(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Trunc:
        emitTrunc(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Reciprocal:
        emitReciprocal(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Sin:
        emitSin(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Cos:
        emitCos(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Exp2:
        emitExp2(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Log2:
        emitLog2(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::Sqrt:
        emitSqrt(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::ReciprocalSqrt:
        emitReciprocalSqrt(inst.getDst(), inst.getOperand(0));
        break;

    // Logical & Bitwise
    case ir::Opcode::Not:
        emitNot(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::BitwiseNot:
        emitBitwiseNot(inst.getDst(), inst.getOperand(0));
        break;
    case ir::Opcode::BitwiseAnd:
        emitBitwiseAnd(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::BitwiseOr:
        emitBitwiseOr(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::BitwiseXor:
        emitBitwiseXor(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::ShiftLeft:
        emitShiftLeft(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::ShiftRight:
        emitShiftRight(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::BitfieldExtract:
        emitBitfieldExtract(inst.getDst(), inst.getOperand(0),
                            inst.getOperand(1), inst.getOperand(2));
        break;

    // Comparison & Selection
    case ir::Opcode::CompareLess:
        emitCompareLess(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::CompareLessOrEqual:
        emitCompareLessOrEqual(inst.getDst(), inst.getOperand(0),
                               inst.getOperand(1));
        break;
    case ir::Opcode::CompareGreater:
        emitCompareGreater(inst.getDst(), inst.getOperand(0),
                           inst.getOperand(1));
        break;
    case ir::Opcode::CompareGreaterOrEqual:
        emitCompareGreaterOrEqual(inst.getDst(), inst.getOperand(0),
                                  inst.getOperand(1));
        break;
    case ir::Opcode::CompareEqual:
        emitCompareEqual(inst.getDst(), inst.getOperand(0), inst.getOperand(1));
        break;
    case ir::Opcode::CompareNotEqual:
        emitCompareNotEqual(inst.getDst(), inst.getOperand(0),
                            inst.getOperand(1));
        break;
    case ir::Opcode::Select:
        emitSelect(inst.getDst(), inst.getOperand(0), inst.getOperand(1),
                   inst.getOperand(2));
        break;

    // Control flow
    case ir::Opcode::BeginIf:
        emitBeginIf(inst.getOperand(0));
        break;
    case ir::Opcode::EndIf:
        emitEndIf();
        break;
    case ir::Opcode::Branch:
        emitBranch(inst.getOperand(0).getLabel());
        break;
    case ir::Opcode::BranchConditional:
        emitBranchConditional(inst.getOperand(0), inst.getOperand(1).getLabel(),
                              inst.getOperand(2).getLabel());
        break;

    // Vector
    case ir::Opcode::VectorExtract:
        emitVectorExtract(inst.getDst(), inst.getOperand(0),
                          inst.getOperand(1).getRawValue<u8>());
        break;
    case ir::Opcode::VectorInsert:
        emitVectorInsert(inst.getDst(), inst.getOperand(0),
                         inst.getOperand(1).getRawValue<u8>());
        break;
    case ir::Opcode::VectorConstruct:
        emitVectorConstruct(inst.getDst(), inst.getOperands());
        break;

    // Texture
    case ir::Opcode::TextureSample:
        emitTextureSample(inst.getDst(), inst.getOperand(0).getRawValue<u32>(),
                          inst.getOperand(1).getRawValue<TextureType>(),
                          inst.getOperand(2).getRawValue<TextureSampleFlags>(),
                          inst.getOperand(3), inst.getOperand(4),
                          inst.getOperand(5), inst.getOperand(6));
        break;
    case ir::Opcode::TextureGather:
        emitTextureGather(inst.getDst(), inst.getOperand(0).getRawValue<u32>(),
                          inst.getOperand(1),
                          inst.getOperand(2).getRawValue<u8>());
        break;
    case ir::Opcode::TextureQueryDimension:
        emitTextureQueryDimension(inst.getDst(),
                                  inst.getOperand(0).getRawValue<u32>(),
                                  inst.getOperand(1).getRawValue<u32>());
        break;

    // Exit
    case ir::Opcode::Exit:
        emitExit();
        break;
    case ir::Opcode::Discard:
        emitDiscard();
        break;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen
