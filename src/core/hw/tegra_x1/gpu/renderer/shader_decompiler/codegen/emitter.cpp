#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/emitter.hpp"

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/module.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen {

void Emitter::Emit(const ir::Module& modul) {
    Start();
    for (const auto& [name, func] : modul.GetFunctions())
        EmitFunction(func);
    Finish();
}

void Emitter::EmitInstruction(const ir::Instruction& inst) {
    switch (inst.GetOpcode()) {
    // Data
    case ir::Opcode::Copy:
        EmitCopy(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Cast:
        EmitCast(inst.GetDst(), inst.GetOperand(0),
                 inst.GetOperand(1).GetRawValue<DataType>());
        break;

    // Arithmetic
    case ir::Opcode::Abs:
        EmitAbs(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Neg:
        EmitNeg(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Add:
        EmitAdd(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::Multiply:
        EmitMultiply(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::Fma:
        EmitFma(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1),
                inst.GetOperand(2));
        break;
    case ir::Opcode::Min:
        EmitMin(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::Max:
        EmitMax(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::Clamp:
        EmitClamp(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1),
                  inst.GetOperand(2));
        break;

    // Math
    case ir::Opcode::IsNan:
        EmitIsNan(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Round:
        EmitRound(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Floor:
        EmitFloor(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Ceil:
        EmitCeil(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Trunc:
        EmitTrunc(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Reciprocal:
        EmitReciprocal(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Sin:
        EmitSin(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Cos:
        EmitCos(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Exp2:
        EmitExp2(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Log2:
        EmitLog2(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Sqrt:
        EmitSqrt(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::ReciprocalSqrt:
        EmitReciprocalSqrt(inst.GetDst(), inst.GetOperand(0));
        break;

    // Logical & Bitwise
    case ir::Opcode::Not:
        EmitNot(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::BitwiseNot:
        EmitBitwiseNot(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::BitwiseAnd:
        EmitBitwiseAnd(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::BitwiseOr:
        EmitBitwiseOr(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::BitwiseXor:
        EmitBitwiseXor(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::ShiftLeft:
        EmitShiftLeft(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::ShiftRight:
        EmitShiftRight(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;

    // Comparison & Selection
    case ir::Opcode::CompareLess:
        EmitCompareLess(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::CompareLessOrEqual:
        EmitCompareLessOrEqual(inst.GetDst(), inst.GetOperand(0),
                               inst.GetOperand(1));
        break;
    case ir::Opcode::CompareGreater:
        EmitCompareGreater(inst.GetDst(), inst.GetOperand(0),
                           inst.GetOperand(1));
        break;
    case ir::Opcode::CompareGreaterOrEqual:
        EmitCompareGreaterOrEqual(inst.GetDst(), inst.GetOperand(0),
                                  inst.GetOperand(1));
        break;
    case ir::Opcode::CompareEqual:
        EmitCompareEqual(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1));
        break;
    case ir::Opcode::CompareNotEqual:
        EmitCompareNotEqual(inst.GetDst(), inst.GetOperand(0),
                            inst.GetOperand(1));
        break;
    case ir::Opcode::Select:
        EmitSelect(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1),
                   inst.GetOperand(2));
        break;

    // Control flow
    case ir::Opcode::BeginIf:
        EmitBeginIf(inst.GetOperand(0));
        break;
    case ir::Opcode::EndIf:
        EmitEndIf();
        break;
    case ir::Opcode::Branch:
        EmitBranch(inst.GetOperand(0).GetLabel());
        break;
    case ir::Opcode::BranchConditional:
        EmitBranchConditional(inst.GetOperand(0), inst.GetOperand(1).GetLabel(),
                              inst.GetOperand(2).GetLabel());
        break;

    // Vector
    case ir::Opcode::VectorExtract:
        EmitVectorExtract(inst.GetDst(), inst.GetOperand(0),
                          inst.GetOperand(1).GetRawValue<u32>());
        break;
    case ir::Opcode::VectorInsert:
        EmitVectorInsert(inst.GetDst(), inst.GetOperand(0),
                         inst.GetOperand(1).GetRawValue<u32>());
        break;
    case ir::Opcode::VectorConstruct:
        EmitVectorConstruct(inst.GetDst(),
                            inst.GetOperand(0).GetRawValue<DataType>(),
                            std::vector(inst.GetOperands().begin() + 1,
                                        inst.GetOperands().end()));
        break;

    // Texture
    case ir::Opcode::TextureSample:
        EmitTextureSample(inst.GetDst(), inst.GetOperand(0).GetRawValue<u32>(),
                          inst.GetOperand(1));
        break;
    case ir::Opcode::TextureRead:
        EmitTextureRead(inst.GetDst(), inst.GetOperand(0).GetRawValue<u32>(),
                        inst.GetOperand(1));
        break;
    case ir::Opcode::TextureGather:
        EmitTextureGather(inst.GetDst(), inst.GetOperand(0).GetRawValue<u32>(),
                          inst.GetOperand(1),
                          inst.GetOperand(2).GetRawValue<u8>());
        break;
    case ir::Opcode::TextureQueryDimension:
        EmitTextureQueryDimension(inst.GetDst(),
                                  inst.GetOperand(0).GetRawValue<u32>(),
                                  inst.GetOperand(1).GetRawValue<u32>());
        break;

    // Exit
    case ir::Opcode::Exit:
        EmitExit();
        break;
    case ir::Opcode::Discard:
        EmitDiscard();
        break;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen
