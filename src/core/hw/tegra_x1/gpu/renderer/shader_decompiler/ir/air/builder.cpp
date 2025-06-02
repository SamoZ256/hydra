#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/air/builder.hpp"

#include "air_signature.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/analyzer/analyzer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::IR::AIR {

// TODO: make this configurable
static constexpr bool FAST_MATH_ENABLED = true;

Builder::Builder(const Analyzer::Analyzer& analyzer, const ShaderType type,
                 const GuestShaderState& state, std::vector<u8>& out_code,
                 ResourceMapping& out_resource_mapping)
    : BuilderBase(analyzer, type, state, out_code, out_resource_mapping),
      types(context) {
    context.setOpaquePointers(false);
}

void Builder::InitializeResourceMapping() {
    for (const auto& [index, size] :
         analyzer.GetMemoryAnalyzer().GetUniformBuffers()) {
        out_resource_mapping.uniform_buffers[index] = index;
    }

    // TODO: storage buffers

    u32 texture_index = 0;
    for (const auto const_buffer_index :
         analyzer.GetMemoryAnalyzer().GetTextures()) {
        out_resource_mapping.textures[const_buffer_index] = texture_index++;
    }

    // TODO: images
}

void Builder::Start() {
    m = new llvm::Module("default", context);

    // TODO: diagnostics handler

    // TODO: move this to the API
    m->setSourceFileName("airconv_generated.metal");
    m->setTargetTriple("air64-apple-macosx14.0.0");
    m->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:"
                     "64-f32:32:32-f64:"
                     "64:64-v16:16:16-v24:32:32-v32:32:32-v48:64:64-v64:64:"
                     "64-v96:128:128-"
                     "v128:128:128-v192:256:256-v256:256:256-v512:512:512-"
                     "v1024:1024:1024-n8:"
                     "16:32");
    m->setSDKVersion(llvm::VersionTuple(14, 0));
    m->addModuleFlag(llvm::Module::ModFlagBehavior::Error, "wchar_size", 4);
    m->addModuleFlag(llvm::Module::ModFlagBehavior::Max, "frame-pointer", 2);
    m->addModuleFlag(llvm::Module::ModFlagBehavior::Max,
                     "air.max_device_buffers", 31);
    m->addModuleFlag(llvm::Module::ModFlagBehavior::Max,
                     "air.max_constant_buffers", 31);
    m->addModuleFlag(llvm::Module::ModFlagBehavior::Max,
                     "air.max_threadgroup_buffers", 31);
    m->addModuleFlag(llvm::Module::ModFlagBehavior::Max, "air.max_textures",
                     128);
    m->addModuleFlag(llvm::Module::ModFlagBehavior::Max,
                     "air.max_read_write_textures", 8);
    m->addModuleFlag(llvm::Module::ModFlagBehavior::Max, "air.max_samplers",
                     16);

    auto createUnsignedInteger = [&](uint32_t s) {
        return llvm::ConstantAsMetadata::get(
            llvm::ConstantInt::get(context, llvm::APInt{32, s, false}));
    };
    auto createString = [&](auto s) { return llvm::MDString::get(context, s); };

    auto airVersion = m->getOrInsertNamedMetadata("air.version");
    airVersion->addOperand(llvm::MDTuple::get(
        context, {createUnsignedInteger(2), createUnsignedInteger(6),
                  createUnsignedInteger(0)}));
    auto airLangVersion = m->getOrInsertNamedMetadata("air.language_version");
    airLangVersion->addOperand(llvm::MDTuple::get(
        context, {createString("Metal"), createUnsignedInteger(3),
                  createUnsignedInteger(0), createUnsignedInteger(0)}));

    auto airCompileOptions = m->getOrInsertNamedMetadata("air.compile_options");
    airCompileOptions->addOperand(llvm::MDTuple::get(
        context, {createString("air.compile.denorms_disable")}));
    airCompileOptions->addOperand(llvm::MDTuple::get(
        context,
        {FAST_MATH_ENABLED ? createString("air.compile.fast_math_enable")
                           : createString("air.compile.fast_math_disable")}));
    airCompileOptions->addOperand(llvm::MDTuple::get(
        context, {createString("air.compile.framebuffer_fetch_enable")}));

    // Builder
    luft::FunctionSignatureBuilder signature_builder;
    InitializeSignature(signature_builder);
    auto [func, meta] =
        signature_builder.CreateFunction("main_", context, *m, 0, false);
    function = func;

    auto prologue_bb = llvm::BasicBlock::Create(context, "prologue", function);
    auto entry_bb = llvm::BasicBlock::Create(context, "entry", function);
    epilogue_bb = llvm::BasicBlock::Create(context, "epilogue", function);
    builder = new llvm::IRBuilder<>(prologue_bb);
    builder->getFastMathFlags().setFast(FAST_MATH_ENABLED);

    std::string stage_name;
    switch (type) {
    case ShaderType::Vertex:
        stage_name = "air.vertex";
        break;
    case ShaderType::Fragment:
        stage_name = "air.fragment";
        break;
    default:
        stage_name = "air.unknown";
        break;
    }
    m->getOrInsertNamedMetadata(stage_name)->addOperand(meta);

    // Registers
    regs_ty = llvm::ArrayType::get(types._int, 256);
    regs_v = builder->CreateAlloca(regs_ty, nullptr, "r");

    // AMEM
    amem_ty = llvm::ArrayType::get(types._int,
                                   0x200); // TODO: what should be the size?
    amem_v = builder->CreateAlloca(amem_ty, nullptr, "a");

    switch (type) {
    case ShaderType::Vertex:
        for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
            const auto vertex_attrib_state = state.vertex_attrib_states[i];
            if (vertex_attrib_state.type == engines::VertexAttribType::None)
                continue;

            // HACK: how are attributes disabled?
            if (vertex_attrib_state.is_fixed)
                continue;

            // TODO: only set if the Rendered backend doesn't support scaled
            // attributes
            bool needs_scaling = (vertex_attrib_state.type ==
                                      engines::VertexAttribType::Sscaled ||
                                  vertex_attrib_state.type ==
                                      engines::VertexAttribType::Uscaled);

            const auto sv = Sv(SvSemantic::UserInOut, i);
            for (u32 c = 0; c < 4; c++) {
                const auto attr = GetA({RZ, 0x80 + i * 0x10 + c * 0x4}, true);
                auto v = LoadSvInput(SvAccess(sv, c));
                if (needs_scaling)
                    v = builder->CreateFPExt(v, types._float); // TODO: what

                builder->CreateStore(v, attr);
            }
        }
        break;
    case ShaderType::Fragment:
#define ADD_INPUT(sv_semantic, index, a_base)                                  \
    {                                                                          \
        for (u32 c = 0; c < 4; c++) {                                          \
            const auto attr = GetA({RZ, a_base + c * 0x4}, true);              \
            auto v = LoadSvInput(SvAccess(Sv(sv_semantic, index), c));         \
            builder->CreateStore(v, attr);                                     \
        }                                                                      \
    }

        ADD_INPUT(SvSemantic::Position, invalid<u8>(), 0x70);
        for (const auto input : analyzer.GetMemoryAnalyzer().GetStageInputs())
            ADD_INPUT(SvSemantic::UserInOut, input, 0x80 + input * 0x10);

#undef ADD_INPUT
        break;
    default:
        break;
    }

    // CMEM
    llvm::ArrayType* cmem_buffer_ty = llvm::ArrayType::get(
        types._int, 0x40); // TODO: what should be the size?
    cmem_ty = llvm::ArrayType::get(cmem_buffer_ty, CONST_BUFFER_BINDING_COUNT);
    cmem_v = builder->CreateAlloca(cmem_ty, nullptr, "c");

    for (const auto& [index, size] :
         analyzer.GetMemoryAnalyzer().GetUniformBuffers()) {
        const auto& buffer = buffers[index];
        u32 u32_count = size / sizeof(u32);
        for (u32 i = 0; i < u32_count; i++) {
            auto v = builder->CreateExtractValue(
                builder->CreateLoad(
                    llvm::ArrayType::get(types._int, size / sizeof(u32)),
                    function->getArg(buffer.first)),
                {i});
            builder->CreateStore(v, GetC({index, RZ, i * sizeof(u32)}, true));
        }
    }

    builder->CreateBr(entry_bb);
    builder->SetInsertPoint(entry_bb);
}

void Builder::Finish() {
    CreateEpilogue();

    delete builder;

    // TODO: remove this
    m->print(llvm::outs(), nullptr);

#ifdef HYDRA_DEBUG
    if (llvm::verifyModule(*m, &llvm::errs()))
        LOG_FATAL(ShaderDecompiler, "LLVM m verification failed");
#endif

    // HACK
    /*
    std::string temp_code;
    llvm::raw_string_ostream temp_stream(temp_code);
    m->print(temp_stream, nullptr);
    delete m;

    std::unique_ptr<llvm::MemoryBuffer> buffer =
        llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(temp_code));
    llvm::SMDiagnostic error;
    auto new_module = llvm::parseIR(buffer->getMemBufferRef(), error, context);
    if (new_module) {
        m = new_module.get();
    } else {
        error.print("llvm-code", llvm::errs());
        throw;
    }
    */

    // RunOptimizationPasses(llvm::OptimizationLevel::O2);

    // TODO: write directly to the output code
    llvm::SmallVector<char, 0> vec;
    llvm::raw_svector_ostream os(vec);
    luft::metallib::MetallibWriter writer;
    writer.Write(*m, os);

    out_code.assign(vec.begin(), vec.end());

    delete m;
}

void Builder::OpExit() { builder->CreateBr(epilogue_bb); }

void Builder::OpMove(reg_t dst, Operand src) {
    builder->CreateStore(GetOperand(src), GetReg(dst, true, src.data_type));
}

void Builder::OpAdd(Operand dst, Operand src1, Operand src2) {
    auto src1_v = GetOperand(src1);
    auto src2_v = GetOperand(src2);
    llvm::Value* res_v;
    if (src1.data_type == DataType::Float)
        res_v = builder->CreateFAdd(src1_v, src2_v, "fadd");
    else
        res_v = builder->CreateAdd(src1_v, src2_v, "add");
    builder->CreateStore(res_v, GetOperand(dst, true));
}

void Builder::OpMultiply(Operand dst, Operand src1, Operand src2) {
    auto src1_v = GetOperand(src1);
    auto src2_v = GetOperand(src2);
    llvm::Value* res_v;
    if (src1.data_type == DataType::Float)
        res_v = builder->CreateFMul(src1_v, src2_v, "fmul");
    else
        res_v = builder->CreateMul(src1_v, src2_v, "mul");
    builder->CreateStore(res_v, GetOperand(dst, true));
}

void Builder::OpFloatFma(reg_t dst, reg_t src1, Operand src2, Operand src3) {
    auto res_v = builder->CreateFAdd(
        GetReg(src1, false, DataType::Float),
        builder->CreateFMul(GetOperand(src2), GetOperand(src3), "fma_fmul"),
        "fma");
    builder->CreateStore(res_v, GetReg(dst, true, DataType::Float));
}

void Builder::OpShiftLeft(reg_t dst, reg_t src, u32 shift) {
    auto res_v = builder->CreateShl(GetReg(src), shift, "shift_left");
    builder->CreateStore(res_v, GetReg(dst, true));
}

void Builder::OpMathFunction(MathFunc func, reg_t dst, reg_t src) {
    auto src_v = GetReg(src, false, DataType::Float);

#define MATH_FUNC(name)                                                        \
    luft::call_float_unary_op(#name, src_v).build(GetAirBuilderContext()).get()

    llvm::Value* res_v;
    // TODO: check
    switch (func) {
    case MathFunc::Cos:
        res_v = MATH_FUNC(cos);
        break;
    case MathFunc::Sin:
        res_v = MATH_FUNC(sin);
        break;
    case MathFunc::Ex2:
        res_v = MATH_FUNC(ex2);
        break;
    case MathFunc::Lg2:
        res_v = MATH_FUNC(log2);
        break;
    case MathFunc::Rcp:
        res_v = builder->CreateFDiv(GetImmediate<f32>(1.0f), src_v);
        break;
    case MathFunc::Rsq:
    case MathFunc::Rcp64h:
        res_v = builder->CreateFDiv(GetImmediate<f32>(1.0f),
                                    src_v); // TODO: correct
        break;
    case MathFunc::Rsq64h:
        res_v = builder->CreateFDiv(GetImmediate<f32>(1.0f),
                                    MATH_FUNC(sqrt)); // TODO: correct?
        break;
    case MathFunc::Sqrt:
        res_v = MATH_FUNC(sqrt);
        break;
    default:
        res_v = nullptr;
        break;
    }

#undef MATH_FUNC

    builder->CreateStore(res_v, GetReg(dst, true, DataType::Float));
}

void Builder::OpLoad(reg_t dst, Operand src) {
    builder->CreateStore(GetOperand(src), GetReg(dst, true, src.data_type));
}

void Builder::OpStore(AMem dst, reg_t src) {
    builder->CreateStore(GetReg(src), GetA(dst, true));
}

void Builder::OpInterpolate(reg_t dst, AMem src) {
    // TODO: interpolate param
    builder->CreateStore(GetA(src), GetReg(dst, true));
}

void Builder::OpTextureSample(reg_t dst0, reg_t dst1, u32 const_buffer_index,
                              reg_t coords_x, reg_t coords_y) {
    llvm::Value* coords_v = llvm::UndefValue::get(types._float2);
    coords_v = builder->CreateInsertElement(
        coords_v, GetReg(coords_x, false, DataType::Float),
        GetImmediate<u32>(0));
    coords_v = builder->CreateInsertElement(
        coords_v, GetReg(coords_y, false, DataType::Float),
        GetImmediate<u32>(0));

    // TODO: don't hardcode the type
    // TODO: why does it return { <4 x float>, i8 }?
    auto res_v = luft::call_sample(
                     luft::MSLTexture{
                         .component_type = luft::msl_float,
                         .memory_access = luft::MemoryAccess::sample,
                         .resource_kind = luft::TextureKind::texture_2d,
                         .resource_kind_logical = luft::TextureKind::texture_2d,
                     },
                     function->getArg(textures[const_buffer_index]),
                     function->getArg(samplers[const_buffer_index]), coords_v,
                     GetImmediate<u32>(0))
                     .build(GetAirBuilderContext())
                     .get();
    res_v = builder->CreateExtractValue(res_v, 0);
    for (u32 i = 0; i < 2; i++)
        builder->CreateStore(builder->CreateExtractElement(res_v, i),
                             GetReg(dst0 + i, true, DataType::Float));
    for (u32 i = 0; i < 2; i++)
        builder->CreateStore(builder->CreateExtractElement(res_v, 2 + i),
                             GetReg(dst1 + i, true, DataType::Float));
}

void Builder::InitializeSignature(
    luft::FunctionSignatureBuilder& signature_builder) {
    // Stage inputs
    switch (type) {
    case ShaderType::Vertex:
        for (u32 i = 0; i < VERTEX_ATTRIB_COUNT; i++) {
            const auto vertex_attrib_state = state.vertex_attrib_states[i];
            if (vertex_attrib_state.type == engines::VertexAttribType::None)
                continue;

            // HACK: how are attributes disabled?
            if (vertex_attrib_state.is_fixed)
                continue;

            luft::InputAttributeComponentType data_type =
                luft::InputAttributeComponentType::Unknown;
            switch (to_data_type(vertex_attrib_state.type)) {
            case DataType::Float:
                data_type = luft::InputAttributeComponentType::Float;
                break;
            case DataType::Int:
                data_type = luft::InputAttributeComponentType::Int;
                break;
            case DataType::UInt:
                data_type = luft::InputAttributeComponentType::Uint;
                break;
            default:
                break;
            }

            u32 arg_index =
                signature_builder.DefineInput(luft::InputVertexStageIn{
                    .attribute = i,
                    .type = data_type,
                    .name = fmt::format("attribute{}", i),
                });
            inputs[Sv(SvSemantic::UserInOut, i)] = arg_index;
        }
        break;
    case ShaderType::Fragment:
        signature_builder.DefineInput(luft::InputPosition{
            .interpolation = luft::Interpolation::center_no_perspective, // TODO
        });
        for (const auto input : analyzer.GetMemoryAnalyzer().GetStageInputs()) {
            // TODO: don't hardcode the type
            const auto type = luft::msl_float4;

            u32 arg_index =
                signature_builder.DefineInput(luft::InputFragmentStageIn{
                    .user = fmt::format("locn{}", input),
                    .type = type,
                    .interpolation =
                        luft::Interpolation::center_no_perspective, // TODO
                    .pull_mode = false,                             // TODO
                });
            inputs[Sv(SvSemantic::UserInOut, input)] = arg_index;
        }
        break;
    default:
        break;
    }

    // Output SVs
    for (const auto sv_semantic : analyzer.GetMemoryAnalyzer().GetOutputSVs()) {
        switch (sv_semantic) {
        case SvSemantic::Position: {
            u32 arg_index = signature_builder.DefineOutput(luft::OutputPosition{
                .type = luft::msl_float4,
            });
            outputs[Sv(SvSemantic::Position)] = arg_index;
            break;
        }
        default:
            LOG_NOT_IMPLEMENTED(ShaderDecompiler, "Output SV semantic {}",
                                sv_semantic);
            break;
        }
    }

    // Stage outputs
    switch (type) {
    case ShaderType::Vertex:
        for (const auto output :
             analyzer.GetMemoryAnalyzer().GetStageOutputs()) {
            // TODO: don't hardcode the type
            const auto type = luft::msl_float4;

            u32 arg_index = signature_builder.DefineOutput(luft::OutputVertex{
                .user = fmt::format("locn{}", output),
                .type = type,
            });
            outputs[Sv(SvSemantic::UserInOut, output)] = arg_index;
        }
        break;
    case ShaderType::Fragment:
        for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_format = state.color_target_formats[i];
            if (color_target_format == TextureFormat::Invalid)
                continue;

            luft::MSLScalerOrVectorType type;
            switch (to_data_type(color_target_format)) {
            case DataType::Float:
                type = luft::msl_float4;
                break;
            case DataType::Int:
                type = luft::msl_int4;
                break;
            case DataType::UInt:
                type = luft::msl_uint4;
                break;
            default:
                LOG_ERROR(ShaderDecompiler,
                          "Unsupported color target format {}",
                          color_target_format);
                type = luft::msl_float4;
                break;
            }
            u32 arg_index =
                signature_builder.DefineOutput(luft::OutputRenderTarget{
                    .dual_source_blending = false, // TODO
                    .index = i,
                    .type = type,
                });
            outputs[Sv(SvSemantic::UserInOut, i)] = arg_index;
        }
        break;
    default:
        break;
    }

    // Uniform buffers
    for (const auto& [index, size] :
         analyzer.GetMemoryAnalyzer().GetUniformBuffers()) {
        // TODO: are the sizes correct?
        auto arg_index =
            signature_builder.DefineInput(luft::ArgumentBindingBuffer{
                .buffer_size = size,
                .location_index = index,
                .array_size = static_cast<u32>(size / sizeof(u32)),
                .memory_access = luft::MemoryAccess::read,
                .address_space = luft::AddressSpace::constant,
                .type = luft::msl_uint,
                .arg_name = fmt::format("ubuff{}", index),
                .raster_order_group = {},
            });
        buffers[index] = {arg_index, size};
    }

    // Storage buffers
    // TODO

    // Textures
    for (const auto const_buffer_index :
         analyzer.GetMemoryAnalyzer().GetTextures()) {
        const auto index = out_resource_mapping.textures[const_buffer_index];
        {
            // TODO: don't hardcode texture type
            u32 arg_index =
                signature_builder.DefineInput(luft::ArgumentBindingTexture{
                    .location_index = index,
                    .array_size = 1,
                    .memory_access = luft::MemoryAccess::sample,
                    .type =
                        luft::MSLTexture{
                            .component_type = luft::msl_float,
                            .memory_access = luft::MemoryAccess::sample,
                            .resource_kind = luft::TextureKind::texture_2d,
                            .resource_kind_logical =
                                luft::TextureKind::texture_2d,
                        },
                    .arg_name = fmt::format("tex{}", index),
                    .raster_order_group = {},
                });
            textures[const_buffer_index] = arg_index;
        }

        {
            u32 arg_index =
                signature_builder.DefineInput(luft::ArgumentBindingSampler{
                    .location_index = index,
                    .array_size = 1,
                    .arg_name = fmt::format("samplr{}", index),
                });
            samplers[const_buffer_index] = arg_index;
        }
    }

    // Images
    // TODO
}

void Builder::CreateEpilogue() {
    builder->SetInsertPoint(epilogue_bb);

    auto ret = builder->CreateAlloca(function->getReturnType());

    // Outputs
    switch (type) {
    case ShaderType::Vertex:
        // TODO: don't hardcode the bit cast type
#define ADD_OUTPUT(sv_semantic, index, a_base)                                 \
    {                                                                          \
        for (u32 c = 0; c < 4; c++) {                                          \
            /* TODO: don't hardcode the type */                                \
            auto attr = GetA({RZ, a_base + c * 0x4}, false, DataType::Float);  \
            StoreSvOutput(ret, SvAccess(Sv(sv_semantic, index), c), attr);     \
        }                                                                      \
    }

        ADD_OUTPUT(SvSemantic::Position, invalid<u8>(), 0x70);
        for (const auto output : analyzer.GetMemoryAnalyzer().GetStageOutputs())
            ADD_OUTPUT(SvSemantic::UserInOut, output, 0x80 + output * 0x10);

#undef ADD_OUTPUT
        break;
    case ShaderType::Fragment:
        for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
            const auto color_target_format = state.color_target_formats[i];
            if (color_target_format == TextureFormat::Invalid)
                continue;

            for (u32 c = 0; c < 4; c++) {
                // TODO: don't hardcode the type
                auto color = GetReg(i * 4 + c, false, DataType::Float);
                StoreSvOutput(ret, SvAccess(Sv(SvSemantic::UserInOut, i), c),
                              color);
            }
        }
        break;
    default:
        break;
    }

    builder->CreateRet(builder->CreateLoad(function->getReturnType(), ret));
}

void Builder::RunOptimizationPasses(llvm::OptimizationLevel opt) {
    /*
            if (!llvm_overwrite.test_and_set()) {
                auto Map = cl::getRegisteredOptions();
                auto InfiniteLoopThreshold =
                    Map["instcombine-infinite-loop-threshold"];
                if (InfiniteLoopThreshold) {
                    reinterpret_cast<cl::opt<unsigned>*>(InfiniteLoopThreshold)
                        ->setValue(1000);
                }
            }
            */

    // Create the analysis managers.
    // These must be declared in this order so that they are destroyed in
    // the correct order due to inter-analysis-manager references.
    llvm::LoopAnalysisManager LAM;
    llvm::FunctionAnalysisManager FAM;
    llvm::CGSCCAnalysisManager CGAM;
    llvm::ModuleAnalysisManager MAM;

    // Create the new pass manager builder.
    // Take a look at the PassBuilder constructor parameters for more
    // customization, e.g. specifying a TargetMachine or various debugging
    // options.
    llvm::PassBuilder PB;

    // Register all the basic analyses with the managers.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);

    llvm::ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(opt);

    llvm::FunctionPassManager FPM;
    // FPM.addPass(llvm::ScalarizerPass());

    MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
    // TODO: remove?
    // MPM.addPass(llvm::VerifierPass());

    // Optimize the IR!
    MPM.run(*m, MAM);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::IR::AIR
