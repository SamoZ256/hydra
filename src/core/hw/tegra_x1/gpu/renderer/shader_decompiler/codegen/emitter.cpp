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
    // Basic
    case ir::Opcode::Copy:
        EmitCopy(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Neg:
        EmitNeg(inst.GetDst(), inst.GetOperand(0));
        break;
    case ir::Opcode::Not:
        EmitNot(inst.GetDst(), inst.GetOperand(0));
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
    case ir::Opcode::ShiftLeft:
        EmitShiftLeft(inst.GetDst(), inst.GetOperand(0),
                      inst.GetOperand(1).GetRawValue<u32>());
        break;
    case ir::Opcode::ShiftRight:
        EmitShiftRight(inst.GetDst(), inst.GetOperand(0),
                       inst.GetOperand(1).GetRawValue<u32>());
        break;
    case ir::Opcode::Cast:
        EmitCast(inst.GetDst(), inst.GetOperand(0),
                 inst.GetOperand(1).GetRawValue<DataType>());
        break;
    case ir::Opcode::Compare:
        EmitCompare(inst.GetDst(),
                    inst.GetOperand(0).GetRawValue<ComparisonOp>(),
                    inst.GetOperand(1), inst.GetOperand(2));
        break;
    case ir::Opcode::Bitwise:
        EmitBitwise(inst.GetDst(), inst.GetOperand(0).GetRawValue<BitwiseOp>(),
                    inst.GetOperand(1), inst.GetOperand(2));
        break;
    case ir::Opcode::Select:
        EmitSelect(inst.GetDst(), inst.GetOperand(0), inst.GetOperand(1),
                   inst.GetOperand(2));
        break;

    // Control flow
    case ir::Opcode::Branch:
        EmitBranch(inst.GetOperand(0).GetLabel());
        break;
    case ir::Opcode::BranchConditional:
        EmitBranchConditional(inst.GetOperand(0), inst.GetOperand(1).GetLabel(),
                              inst.GetOperand(2).GetLabel());
        break;
    case ir::Opcode::BeginIf:
        EmitBeginIf(inst.GetOperand(0));
        break;
    case ir::Opcode::EndIf:
        EmitEndIf();
        break;

    // Math
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
    case ir::Opcode::MathFunction:
        EmitMathFunction(inst.GetDst(),
                         inst.GetOperand(0).GetRawValue<MathFunc>(),
                         inst.GetOperand(1));
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

    // Special
    case ir::Opcode::Exit:
        EmitExit();
        break;
    case ir::Opcode::Discard:
        EmitDiscard();
        break;
    case ir::Opcode::TextureSample:
        EmitTextureSample(inst.GetDst(), inst.GetOperand(0).GetRawValue<u32>(),
                          inst.GetOperand(1));
        break;
    case ir::Opcode::TextureRead:
        EmitTextureRead(inst.GetDst(), inst.GetOperand(0).GetRawValue<u32>(),
                        inst.GetOperand(1));
        break;
    case ir::Opcode::TextureQueryDimension:
        EmitTextureQueryDimension(inst.GetDst(),
                                  inst.GetOperand(0).GetRawValue<u32>(),
                                  inst.GetOperand(1).GetRawValue<u32>());
        break;
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen
