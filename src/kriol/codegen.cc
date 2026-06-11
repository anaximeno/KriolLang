#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Bitcode/BitcodeReader.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/MemoryBuffer.h>

#include <llvm/Support/Program.h>
#include <llvm/Support/Path.h>

#if KRIOL_USE_EMBEDDED_LLD

#include <lld/Common/Driver.h>
LLD_HAS_DRIVER(wasm)

#endif

#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <initializer_list>

#include "../../include/kriol/codegen.hh"
#include "../../include/kriol/type_utils.hh"

// NOTE: these includes below are generated and
// injected in compile time by the build system.
#include "kriol_runtime_native_gc.bc.h"
#include "libgc_native.h"

#if KRIOL_ENABLE_WASM

#if KRIOL_WASI_ENABLE_GC
#include "kriol_runtime_wasm32_wasi_gc.bc.h"
#include "libgc_wasm32_wasi.h"
#else
#include "kriol_runtime_wasm32_wasi_nogc.bc.h"
#endif

#include "wasi_crt1_command.o.h"
#include "wasi_libc.a.h"
#include "wasi_libm.a.h"
#include "wasi_builtins.a.h"

#endif


// Interpret backslash escapes in a raw string (without surrounding quotes).
static std::string processEscapes(const std::string& raw) {
    std::string out;
    out.reserve(raw.size());
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] == '\\' && i + 1 < raw.size()) {
            switch (raw[++i]) {
                case 'n':  out += '\n'; break;
                case 't':  out += '\t'; break;
                case 'r':  out += '\r'; break;
                case '\\': out += '\\'; break;
                case '"':  out += '"';  break;
                case '\'': out += '\''; break;
                case '0':  out += '\0'; break;
                case '{':  out += '{';  break;
                case '}':  out += '}';  break;
                default:   out += '\\'; out += raw[i]; break;
            }
        } else {
            out += raw[i];
        }
    }
    return out;
}

// Returns the printf format specifier for a Kriol type.
static const char* fmtSpec(const std::string& kriolType) {
    if (kriolType == "nter") return "%lld";
    if (kriolType == "num")  return "%g";
    return "%s"; // textu / fallback
}

// Reverse-map an LLVM type to the corresponding Kriol type string.
static std::string llvmTypeToKriol(llvm::Type* ty) {
    if (ty->isDoubleTy())    return "num";
    if (ty->isIntegerTy(64)) return "nter";
    if (ty->isIntegerTy(1))  return "bool";
    return "textu"; // pointer / fallback
}

namespace {

using kriol::typeutils::arrayElementType;
using kriol::typeutils::parseType;
using kriol::ast::CodegenTarget;

struct EmbeddedBlob {
    const unsigned char* Data;
    unsigned int Len;
};

struct TargetResources {
    std::string Triple;
    EmbeddedBlob Runtime;
    EmbeddedBlob GcArchive;
};

struct WasiLinkPlan {
    std::string LinkerPath;
    std::string OutputPath;
    std::vector<std::string> Args;
};

struct WasiInputs {
    std::string Crt1Command;
    std::string Libc;
    std::string Libm;
    std::string Builtins;
};

static void initializeTargets() {
    static bool initialized = false;
    if (initialized) return;

    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmParser();
    llvm::InitializeNativeTargetAsmPrinter();

#if KRIOL_ENABLE_WASM
    LLVMInitializeWebAssemblyTargetInfo();
    LLVMInitializeWebAssemblyTarget();
    LLVMInitializeWebAssemblyTargetMC();
    LLVMInitializeWebAssemblyAsmPrinter();
#endif

    initialized = true;
}

static std::string findProgram(std::initializer_list<const char*> names,
                               const std::string& description) {
    for (const char* name : names) {
        auto path = llvm::sys::findProgramByName(name);
        if (path) return *path;
    }
    throw std::runtime_error("The " + description + " could not be found in the system.");
}

#if KRIOL_USE_EMBEDDED_LLD
static std::string findProgramOptional(std::initializer_list<const char*> names) {
    for (const char* name : names) {
        auto path = llvm::sys::findProgramByName(name);
        if (path) return *path;
    }
    return {};
}
#endif

static std::string writeTempBlob(const char* stem,
                                 const char* suffix,
                                 EmbeddedBlob blob) {
    llvm::SmallString<128> tempPath;
    std::error_code ec = llvm::sys::fs::createTemporaryFile(stem, suffix, tempPath);
    if (ec) {
        throw std::runtime_error("Failed to allocate temporary file: " + ec.message());
    }

    llvm::raw_fd_ostream out(tempPath, ec, llvm::sys::fs::OF_None);
    if (ec) {
        throw std::runtime_error("Failed to open temporary file: " + ec.message());
    }

    out.write(reinterpret_cast<const char*>(blob.Data), blob.Len);
    out.close();
    return std::string(tempPath.str());
}

static void runProgram(const std::string& program,
                       const std::vector<std::string>& ownedArgs,
                       const std::string& failurePrefix) {
    std::vector<llvm::StringRef> args;
    args.reserve(ownedArgs.size());
    for (const auto& arg : ownedArgs)
        args.push_back(arg);

    std::string err;
    bool execFailed = false;
    int ret = llvm::sys::ExecuteAndWait(
        program, args, std::nullopt, {}, 0, 0, &err, &execFailed
    );

    if (execFailed || ret != 0)
        throw std::runtime_error(failurePrefix + err);
}

static WasiLinkPlan buildWasiLinkPlan(const std::string& objPath,
                                      const std::string& tempLibGc,
                                      const WasiInputs& wasiInputs,
                                      const std::string& outputPath) {
#if KRIOL_USE_EMBEDDED_LLD
    std::string wasmLdPath = findProgramOptional({"wasm-ld-20", "wasm-ld-19", "wasm-ld"});
    std::string linkerArg0 = wasmLdPath.empty() ? "wasm-ld" : wasmLdPath;
#else
    std::string wasmLdPath = findProgram(
        {"wasm-ld-20", "wasm-ld-19", "wasm-ld"},
        "WASI linker 'wasm-ld-20', 'wasm-ld-19', or 'wasm-ld'"
    );
    std::string linkerArg0 = wasmLdPath;
#endif
    WasiLinkPlan plan;
    plan.LinkerPath = wasmLdPath;
    plan.OutputPath = outputPath;
    plan.Args = {
        linkerArg0,
        "-m",
        "wasm32",
        "--export=__main_argc_argv",
        wasiInputs.Crt1Command,
        objPath,
    };
    if (!tempLibGc.empty())
        plan.Args.push_back(tempLibGc);
    plan.Args.insert(plan.Args.end(), {
        wasiInputs.Libm,
        wasiInputs.Libc,
        wasiInputs.Builtins,
        "-o",
        outputPath
    });

    return plan;
}

static void linkWasmWithWasmLd(const WasiLinkPlan& plan) {
    if (plan.LinkerPath.empty())
        throw std::runtime_error("The WASI linker fallback is not available.");
    runProgram(plan.LinkerPath, plan.Args, "Failure in the final WASI linkage: ");
}

#if KRIOL_USE_EMBEDDED_LLD
static void linkWasmWithEmbeddedLld(const WasiLinkPlan& plan) {
    std::vector<const char*> args;
    args.reserve(plan.Args.size());
    for (const auto& arg : plan.Args)
        args.push_back(arg.c_str());

    std::string stdoutText;
    std::string stderrText;
    llvm::raw_string_ostream stdoutStream(stdoutText);
    llvm::raw_string_ostream stderrStream(stderrText);

    lld::DriverDef drivers[] = {
        {lld::Wasm, &lld::wasm::link}
    };
    lld::Result result = lld::lldMain(args, stdoutStream, stderrStream, drivers);
    stdoutStream.flush();
    stderrStream.flush();

    if (result.retCode != 0 || !result.canRunAgain) {
        std::string detail = stderrText.empty() ? stdoutText : stderrText;
        if (!result.canRunAgain && detail.empty())
            detail = "LLD reported that it cannot be called again.";
        throw std::runtime_error("Failure in the embedded WASI linkage: " + detail);
    }
}
#endif

static void linkWasm(const WasiLinkPlan& plan) {
#if KRIOL_USE_EMBEDDED_LLD
    try {
        linkWasmWithEmbeddedLld(plan);
        return;
    } catch (const std::exception&) {
        llvm::sys::fs::remove(plan.OutputPath);
    }
#endif
    linkWasmWithWasmLd(plan);
}

#if KRIOL_ENABLE_WASM
static WasiInputs writeWasiInputs() {
    return WasiInputs{
        writeTempBlob(
            "kriol_wasi_crt1_command",
            "o",
            EmbeddedBlob{wasi_crt1_command_o, wasi_crt1_command_o_len}
        ),
        writeTempBlob(
            "kriol_wasi_libc",
            "a",
            EmbeddedBlob{wasi_libc_a, wasi_libc_a_len}
        ),
        writeTempBlob(
            "kriol_wasi_libm",
            "a",
            EmbeddedBlob{wasi_libm_a, wasi_libm_a_len}
        ),
        writeTempBlob(
            "kriol_wasi_builtins",
            "a",
            EmbeddedBlob{wasi_builtins_a, wasi_builtins_a_len}
        )
    };
}

static void removeWasiInputs(const WasiInputs& inputs) {
    if (!inputs.Crt1Command.empty()) llvm::sys::fs::remove(inputs.Crt1Command);
    if (!inputs.Libc.empty()) llvm::sys::fs::remove(inputs.Libc);
    if (!inputs.Libm.empty()) llvm::sys::fs::remove(inputs.Libm);
    if (!inputs.Builtins.empty()) llvm::sys::fs::remove(inputs.Builtins);
}
#endif

static TargetResources selectTargetResources(CodegenTarget target) {
    switch (target) {
        case CodegenTarget::Native:
            return TargetResources{
                llvm::sys::getDefaultTargetTriple(),
                EmbeddedBlob{kriol_runtime_native_gc_bc, kriol_runtime_native_gc_bc_len},
                EmbeddedBlob{libgc_native_a, libgc_native_a_len}
            };
        case CodegenTarget::Wasm32Wasi:
#if KRIOL_ENABLE_WASM
#if KRIOL_WASI_ENABLE_GC
            return TargetResources{
                KRIOL_WASI_TARGET,
                EmbeddedBlob{kriol_runtime_wasm32_wasi_gc_bc, kriol_runtime_wasm32_wasi_gc_bc_len},
                EmbeddedBlob{libgc_wasm32_wasi_a, libgc_wasm32_wasi_a_len}
            };
#else
            return TargetResources{
                KRIOL_WASI_TARGET,
                EmbeddedBlob{kriol_runtime_wasm32_wasi_nogc_bc, kriol_runtime_wasm32_wasi_nogc_bc_len},
                EmbeddedBlob{nullptr, 0}
            };
#endif
#else
            throw std::runtime_error("This build of the compiler was built without the experimental 'wasm32-wasi' support.");
#endif
    }

    throw std::runtime_error("Unsupported codegen target.");
}

static void linkRuntimeBitcode(llvm::Module& module,
                               llvm::LLVMContext& context,
                               EmbeddedBlob runtimeBlob) {
    llvm::StringRef bcData(reinterpret_cast<const char*>(runtimeBlob.Data), runtimeBlob.Len);
    auto memBuf = llvm::MemoryBuffer::getMemBuffer(bcData, "kriol_runtime", false);

    auto expectedMod = llvm::parseBitcodeFile(*memBuf, context);
    if (!expectedMod) {
        std::string errDetail = llvm::toString(expectedMod.takeError());
        throw std::runtime_error("Failed to parse runtime bitcode: " + errDetail);
    }

    bool linkErr = llvm::Linker::linkModules(module, std::move(expectedMod.get()));
    if (linkErr)
        throw std::runtime_error("Failed to merge runtime bitcode into the main module.");
}

static void emitObjectFile(llvm::Module& module,
                           const std::string& targetTriple,
                           const std::string& objPath) {
    module.setTargetTriple(targetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(targetTriple, Error);
    if (!Target)
        throw std::runtime_error("Failed to find Target: " + Error);

    auto CPU = "generic";
    auto Features = "";
    llvm::TargetOptions opt;
    auto RM = std::optional<llvm::Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(targetTriple, CPU, Features, opt, RM);

    module.setDataLayout(TargetMachine->createDataLayout());

    std::error_code EC;
    llvm::raw_fd_ostream dest(objPath, EC, llvm::sys::fs::OF_None);
    if (EC)
        throw std::runtime_error("The object file could not be opened: " + EC.message());

    llvm::legacy::PassManager pass;
    auto FileType = llvm::CodeGenFileType::ObjectFile;
    if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
        throw std::runtime_error("The TargetMachine cannot output a file of such type.");

    pass.run(module);
    dest.flush();
}

static std::string writeGcArchive(CodegenTarget target, EmbeddedBlob gcBlob) {
    if (!gcBlob.Data || gcBlob.Len == 0)
        return {};

    const char* stem = target == CodegenTarget::Wasm32Wasi
        ? "embedded_libgc_wasm32_wasi"
        : "embedded_libgc_native";
    return writeTempBlob(stem, "a", gcBlob);
}

static void linkNativeExecutable(const std::string& objPath,
                                 const std::string& tempLibGc,
                                 const std::string& outputPath) {
    std::string ccPath = findProgram({"clang", "cc"}, "linker 'clang' or 'cc'");
    std::vector<std::string> linkArgs = {
        ccPath,
        "-no-pie",
        objPath
    };
    if (!tempLibGc.empty())
        linkArgs.push_back(tempLibGc);
    linkArgs.push_back("-o");
    linkArgs.push_back(outputPath);
    linkArgs.push_back("-lm");
    runProgram(ccPath, linkArgs, "Failure in the final linkage: ");
}

}

using namespace kriol::ast;

CodeGenVisitor::CodeGenVisitor(const std::string& moduleName)
    : Mod(std::make_unique<llvm::Module>(moduleName, Context)),
      Builder(std::make_unique<llvm::IRBuilder<>>(Context))
{
    initializeTargets();
}

llvm::Type* CodeGenVisitor::mapType(const std::string& t) {
    auto parsed = parseType(t);
    if (parsed && parsed->isArray()) {
        llvm::Type* current = mapType(parsed->Base);
        for (auto it = parsed->ArrayDims.rbegin(); it != parsed->ArrayDims.rend(); ++it)
            current = llvm::ArrayType::get(current, *it);
        return current;
    }

    if (t == "num")   return llvm::Type::getDoubleTy(Context);
    if (t == "nter")  return llvm::Type::getInt64Ty(Context);
    if (t == "bool")  return llvm::Type::getInt1Ty(Context);
    if (t == "textu") return llvm::PointerType::getUnqual(Context);
    if (t == "vaziu") return llvm::Type::getVoidTy(Context);

    return llvm::PointerType::getUnqual(Context);
}

llvm::AllocaInst* CodeGenVisitor::createEntryAlloca(
        llvm::Function* fn, const std::string& name, llvm::Type* ty) {
    llvm::IRBuilder<> tmp(&fn->getEntryBlock(), fn->getEntryBlock().begin());
    return tmp.CreateAlloca(ty, nullptr, name);
}

llvm::AllocaInst* CodeGenVisitor::lookupVar(const std::string& name) {
    for (auto it = Scopes.rbegin(); it != Scopes.rend(); ++it) {
        auto found = it->find(name);
        if (found != it->end()) return found->second;
    }
    return nullptr;
}

llvm::GlobalVariable* CodeGenVisitor::lookupGlobal(const std::string& name) {
    auto it = GlobalVars.find(name);
    return it != GlobalVars.end() ? it->second : nullptr;
}

llvm::Function* CodeGenVisitor::getOrDeclarePrintf() {
    if (auto* fn = Mod->getFunction("printf")) return fn;
    auto* ptrTy  = llvm::PointerType::getUnqual(Context);
    auto* ftype  = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(Context), {ptrTy}, /*isVarArg=*/true);
    return llvm::Function::Create(
        ftype, llvm::Function::ExternalLinkage, "printf", *Mod);
}

llvm::Value* CodeGenVisitor::coerce(llvm::Value* v, llvm::Type* targetTy) {
    llvm::Type* srcTy = v->getType();
    if (srcTy == targetTy) return v; // identity

    auto* doubleTy = llvm::Type::getDoubleTy(Context);
    auto* i64Ty    = llvm::Type::getInt64Ty(Context);
    auto* i1Ty     = llvm::Type::getInt1Ty(Context);

    // nter (i64) -> num (double)
    if (srcTy == i64Ty && targetTy == doubleTy)
        return Builder->CreateSIToFP(v, doubleTy, "conv");
    // bool (i1) -> nter (i64)
    if (srcTy == i1Ty && targetTy == i64Ty)
        return Builder->CreateZExt(v, i64Ty, "conv");
    // bool (i1) -> num (double)
    if (srcTy == i1Ty && targetTy == doubleTy)
        return Builder->CreateUIToFP(v, doubleTy, "conv");
    // num (double) -> nter (i64)
    if (srcTy == doubleTy && targetTy == i64Ty)
        return Builder->CreateFPToSI(v, i64Ty, "conv");
    // any integer widening (e.g. i1 -> i64 via general int path)
    if (srcTy->isIntegerTy() && targetTy->isIntegerTy())
        return Builder->CreateSExtOrTrunc(v, targetTy, "conv");

    throw std::runtime_error("Unsupported implicit type conversion");
}

llvm::Value* CodeGenVisitor::toBool(llvm::Value* v) {
    if (v->getType()->isPointerTy())
        throw std::runtime_error("Cannot use non truthy value as a condition");
    if (v->getType()->isIntegerTy(1)) return v;
    if (v->getType()->isDoubleTy())
        return Builder->CreateFCmpONE(
            v, llvm::ConstantFP::get(Context, llvm::APFloat(0.0)), "booltmp");
    return Builder->CreateICmpNE(
        v, llvm::ConstantInt::get(v->getType(), 0), "booltmp");
}

std::string CodeGenVisitor::emitIR() {
    std::string buf;
    llvm::raw_string_ostream os(buf);
    Mod->print(os, nullptr);
    return buf;
}

void CodeGenVisitor::emitNative(const std::string& outputPath) {
    emit(outputPath, {});
}

std::vector<unsigned char> CodeGenVisitor::emitToMemory(const EmitOptions& options) {
    llvm::SmallString<128> outputPath;
    std::error_code ec = llvm::sys::fs::createTemporaryFile(
        options.Target == CodegenTarget::Wasm32Wasi ? "kriol_wasm_output" : "kriol_output",
        options.Target == CodegenTarget::Wasm32Wasi ? "wasm" : "out",
        outputPath
    );
    if (ec)
        throw std::runtime_error("Failed to allocate temporary output file: " + ec.message());

    std::string outputPathStr(outputPath.str());
    try {
        emit(outputPathStr, options);

        auto outputBuffer = llvm::MemoryBuffer::getFile(outputPathStr);
        if (!outputBuffer)
            throw std::runtime_error("Failed to read compiler output: " + outputBuffer.getError().message());

        llvm::StringRef bytes = outputBuffer.get()->getBuffer();
        std::vector<unsigned char> result(bytes.bytes_begin(), bytes.bytes_end());
        llvm::sys::fs::remove(outputPathStr);
        return result;
    } catch (...) {
        llvm::sys::fs::remove(outputPathStr);
        throw;
    }
}

void CodeGenVisitor::emit(const std::string& outputPath, const EmitOptions& options) {
    std::string objPath = outputPath + ".o";
    TargetResources resources = selectTargetResources(options.Target);

    linkRuntimeBitcode(*Mod, Context, resources.Runtime);
    emitObjectFile(*Mod, resources.Triple, objPath);
    std::string tempLibGc = writeGcArchive(options.Target, resources.GcArchive);
#if KRIOL_ENABLE_WASM
    WasiInputs wasiInputs;
    if (options.Target == CodegenTarget::Wasm32Wasi)
        wasiInputs = writeWasiInputs();
#endif

    try {
        if (options.Target == CodegenTarget::Wasm32Wasi) {
#if KRIOL_ENABLE_WASM
            linkWasm(buildWasiLinkPlan(objPath, tempLibGc, wasiInputs, outputPath));
#else
            throw std::runtime_error("kriol was built without wasm32-wasi support.");
#endif
        } else {
            linkNativeExecutable(objPath, tempLibGc, outputPath);
        }
    } catch (...) {
        std::remove(objPath.c_str());
        if (!tempLibGc.empty()) llvm::sys::fs::remove(tempLibGc);
#if KRIOL_ENABLE_WASM
        if (options.Target == CodegenTarget::Wasm32Wasi)
            removeWasiInputs(wasiInputs);
#endif
        throw;
    }

    std::remove(objPath.c_str());
    if (!tempLibGc.empty()) llvm::sys::fs::remove(tempLibGc);
#if KRIOL_ENABLE_WASM
    if (options.Target == CodegenTarget::Wasm32Wasi)
        removeWasiInputs(wasiInputs);
#endif
}

void CodeGenVisitor::visit(VarDeclSttmt& node) {
    TypeTable[node.Name] = node.Type;
    if (node.IsParam) return; // params are handled inside FuncDeclSttmt

    llvm::Type* ty = mapType(node.Type);

    // Module scope: CurrentFunction is null when we're outside any function
    if (!CurrentFunction) {
        llvm::Constant* init = llvm::Constant::getNullValue(ty);

        auto* gv = new llvm::GlobalVariable(
            *Mod,
            ty,
            /*isConstant=*/false,
            llvm::GlobalValue::InternalLinkage,
            init,
            node.Name
        );

        GlobalVars[node.Name] = gv;

        // Defer any initializer expression; emitted as stores at the top of inisiu
        if (node.Value)
            DeferredGlobalInits.push_back({gv, node.Value.get()});

        LastValue = nullptr;
        return;
    }

    // Function scope
    llvm::AllocaInst* alloca = createEntryAlloca(CurrentFunction, node.Name, ty);
    declareVar(node.Name, alloca);

    if (node.IsArray) {
        if (node.Value) {
            auto* arrTy = llvm::dyn_cast<llvm::ArrayType>(ty);
            if (!arrTy) throw std::runtime_error("internal error: array variable '" + node.Name + "' has non-array LLVM type");
            auto* elemTy = arrTy->getArrayElementType();

            if (auto* initLit = dynamic_cast<ArrayLiteralExpr*>(node.Value.get())) {
                for (size_t i = 0; i < initLit->Elements.size(); ++i) {
                    initLit->Elements[i]->accept(*this);
                    if (!LastValue) continue;
                    llvm::Value* value = LastValue;
                    if (value->getType() != elemTy) value = coerce(value, elemTy);
                    auto* index = llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), i);
                    llvm::Value* elemPtr = createArrayElementPtr(alloca, ty, index);
                    Builder->CreateStore(value, elemPtr);
                }
            } else if (auto* initRep = dynamic_cast<ArrayRepeatExpr*>(node.Value.get())) {
                initRep->Fill->accept(*this);
                llvm::Value* fillVal = LastValue ? coerce(LastValue, elemTy)
                                                 : llvm::Constant::getNullValue(elemTy);
                for (size_t i = 0; i < initRep->Count; ++i) {
                    auto* index = llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), i);
                    llvm::Value* elemPtr = createArrayElementPtr(alloca, ty, index);
                    Builder->CreateStore(fillVal, elemPtr);
                }
            } else {
                throw std::runtime_error("array variable '" + node.Name + "' requires array literal or repeat initializer");
            }
        }
    } else if (node.Value) {
        node.Value->accept(*this);
        if (LastValue) {
            LastValue = coerce(LastValue, ty);
            Builder->CreateStore(LastValue, alloca);
        }
    }
    LastValue = nullptr;
}

llvm::Function* CodeGenVisitor::getOrDeclareKriolCheckBounds() {
    if (auto* fn = Mod->getFunction("__kriol_check_bounds")) return fn;
    auto* voidTy = llvm::Type::getVoidTy(Context);
    auto* i64Ty  = llvm::Type::getInt64Ty(Context);
    auto* i32Ty  = llvm::Type::getInt32Ty(Context);
    auto* ftype  = llvm::FunctionType::get(voidTy, {i64Ty, i64Ty, i32Ty}, false);
    return llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "__kriol_check_bounds", *Mod);
}

llvm::Value* CodeGenVisitor::getArrayStorage(const std::string& name) {
    if (auto* alloca = lookupVar(name)) return alloca;
    if (auto* gv = lookupGlobal(name)) return gv;
    return nullptr;
}

llvm::Value* CodeGenVisitor::createArrayElementPtr(llvm::Value* storage,
                                                   llvm::Type* arrayTy,
                                                   llvm::Value* index) {
    auto* zero = llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), 0);
    return Builder->CreateGEP(arrayTy, storage, {zero, index}, "array.elem.ptr");
}

void CodeGenVisitor::visit(ArrayAccessExpr& node) {
    auto* storage = getArrayStorage(node.Name);
    if (!storage) { LastValue = nullptr; return; }

    llvm::Type* storageTy = nullptr;
    if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(storage)) storageTy = alloca->getAllocatedType();
    else if (auto* gv = llvm::dyn_cast<llvm::GlobalVariable>(storage)) storageTy = gv->getValueType();
    if (!storageTy || !storageTy->isArrayTy()) { LastValue = nullptr; return; }

    if (node.Index) node.Index->accept(*this);
    if (!LastValue) { LastValue = nullptr; return; }

    auto* i64Ty = llvm::Type::getInt64Ty(Context);
    llvm::Value* idx = LastValue->getType() == i64Ty ? LastValue : coerce(LastValue, i64Ty);

    auto* sizeConst = llvm::ConstantInt::get(i64Ty, storageTy->getArrayNumElements());
    auto* lineConst = llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), node.LineNum);
    Builder->CreateCall(getOrDeclareKriolCheckBounds(), {idx, sizeConst, lineConst});

    llvm::Value* elemPtr = createArrayElementPtr(storage, storageTy, idx);
    LastValue = Builder->CreateLoad(storageTy->getArrayElementType(), elemPtr, node.Name + "[idx]");
}

void CodeGenVisitor::visit(ArrayLiteralExpr& node) {
    // Array literals are consumed in VarDeclSttmt initializers; they do not
    // directly lower to a first-class runtime value in this M4 slice.
    LastValue = nullptr;
}

void CodeGenVisitor::visit(ArrayRepeatExpr& node) {
    // Repeat expresssions are consumed in VarDeclSttmt initializers.
    LastValue = nullptr;
}

void CodeGenVisitor::forwardDeclareFunc(ast::FuncDeclSttmt& node) {
    bool isMain = (node.Name == "inisiu");
    // NOTE: WASI's crt1-command.o (wasi-sdk >= 16 or so) expects the user entry point
    // as __main_argc_argv(i32, ptr) rather than main(). "main" exists in libc as
    // a weak wrapper that calls __main_argc_argv, but only if the user doesn't
    // define their own main — which we do, causing the stub to win.
    // When eventually argc/argv is exposed to the Kriol language, this stays the same.
    std::string name = isMain
        ? (CurrentTarget == CodegenTarget::Wasm32Wasi ? "__main_argc_argv" : "main")
        : node.Name;

    // already declared
    if (Mod->getFunction(name)) return;

    std::vector<llvm::Type*> paramTypes;
    if (isMain && (CurrentTarget == CodegenTarget::Wasm32Wasi)) {
        paramTypes.push_back(llvm::Type::getInt32Ty(Context));
        paramTypes.push_back(llvm::PointerType::getUnqual(Context));
    } else if (node.Args) {
        for (auto& arg : node.Args->Args)
            paramTypes.push_back(mapType(arg->Type));
    }

    llvm::Type* retTy = isMain
        ? llvm::Type::getInt32Ty(Context)
        : mapType(node.Type);

    auto* ftype = llvm::FunctionType::get(retTy, paramTypes, false);
    llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, name, *Mod);
}

void CodeGenVisitor::visit(BlockSttmt& node) {
    // At program root forward-declare all user functions
    // so that forward calls and mutual recursion resolve in codegen.
    if (!CurrentFunction) {
        for (auto& s : node.SttmtList)
            if (auto* fn = dynamic_cast<FuncDeclSttmt*>(s.get()))
                forwardDeclareFunc(*fn);

        // Also visit all top-level variable declarations first so deferred
        // global initializers are complete before main is emitted.
        for (auto& s : node.SttmtList)
            if (auto* v = dynamic_cast<VarDeclSttmt*>(s.get()))
                v->accept(*this);
    }

    pushScope();
    for (auto& s : node.SttmtList) {
        if (!s) continue;
        if (!CurrentFunction && dynamic_cast<VarDeclSttmt*>(s.get())) continue;
        s->accept(*this);
    }
    popScope();
}

void CodeGenVisitor::visit(FuncArgs& node) {
    // Handled inside FuncDeclSttmt
}

// Declare (or retrieve) __kriol_gc_init -> void __kriol_gc_init(void)
static llvm::Function* getOrDeclareKriolGcInit(llvm::Module& Mod, llvm::LLVMContext& Context)
{
    if (auto* fn = Mod.getFunction("__kriol_gc_init")) return fn;
    auto* voidTy = llvm::Type::getVoidTy(Context);
    auto* ftype  = llvm::FunctionType::get(voidTy, {}, false);
    return llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "__kriol_gc_init", Mod);
}

void CodeGenVisitor::visit(FuncDeclSttmt& node) {
    bool isMain = (node.Name == "inisiu");
    // NOTE: WASI's crt1-command.o (wasi-sdk >= 16 or so) expects the user entry point
    // as __main_argc_argv(i32, ptr) rather than main(). "main" exists in libc as
    // a weak wrapper that calls __main_argc_argv, but only if the user doesn't
    // define their own main — which we do, causing the stub to win.
    // When eventually argc/argv is exposed to the Kriol language, this stays the same.
    std::string name = isMain
        ? (CurrentTarget == CodegenTarget::Wasm32Wasi ? "__main_argc_argv" : "main")
        : node.Name;

    std::vector<llvm::Type*> paramTypes;
    if (isMain && (CurrentTarget == CodegenTarget::Wasm32Wasi)) {
        paramTypes.push_back(llvm::Type::getInt32Ty(Context));
        paramTypes.push_back(llvm::PointerType::getUnqual(Context));
    } else if (node.Args) {
        for (auto& arg : node.Args->Args)
            paramTypes.push_back(mapType(arg->Type));
    }

    llvm::Type* retTy = isMain
        ? llvm::Type::getInt32Ty(Context)
        : mapType(node.Type);

    // Get the function if it was already forward-declared, otherwise create it now.
    auto* fn = Mod->getFunction(name);

    if (!fn) {
        auto* ftype = llvm::FunctionType::get(retTy, paramTypes, false);
        fn = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, name, *Mod);
    }

    if (isMain && (CurrentTarget == CodegenTarget::Wasm32Wasi)) {
        auto it = fn->arg_begin();
        it->setName("argc"); ++it;
        it->setName("argv");
    } else if (node.Args) {
        size_t i = 0;
        for (auto& llvmArg : fn->args())
            llvmArg.setName(node.Args->Args[i++]->Name);
    }

    auto* entry = llvm::BasicBlock::Create(Context, "entry", fn);
    Builder->SetInsertPoint(entry);
    CurrentFunction = fn;

    pushScope();
    size_t nodeArgCount = (node.Args ? node.Args->Args.size() : 0);
    if (nodeArgCount > 0) {
        size_t i = 0;
        for (auto& llvmArg : fn->args()) {
            if (i >= nodeArgCount) break; // XXX: skip hidden WASI argc/argv
            auto& p = node.Args->Args[i++];
            TypeTable[p->Name] = p->Type;
            auto* a = createEntryAlloca(fn, p->Name, llvmArg.getType());
            Builder->CreateStore(&llvmArg, a);
            declareVar(p->Name, a);
        }
    }

    // Initialise the Boehm GC as the very first thing in main.
    if (isMain) {
        auto* gcInit = getOrDeclareKriolGcInit(*Mod, Context);
        Builder->CreateCall(gcInit, {});
    }

    // Emit deferred global initializers at the top of inisiu (main)
    if (isMain && !DeferredGlobalInits.empty()) {
        for (auto& di : DeferredGlobalInits) {
            di.InitExpr->accept(*this);
            if (LastValue) {
                LastValue = coerce(LastValue, di.Var->getValueType());
                Builder->CreateStore(LastValue, di.Var);
            }
        }
        DeferredGlobalInits.clear();
    }

    if (node.Body) node.Body->accept(*this);
    popScope();

    // Auto-return if the current block has no terminator
    if (!Builder->GetInsertBlock()->getTerminator()) {
        if (isMain)
            Builder->CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), 0));
        else if (retTy->isVoidTy())
            Builder->CreateRetVoid();
        else {
            // If every branch already returned, the current block is dead code
            // with no predecessors (but it still needs a valid LLVM terminator).
            llvm::BasicBlock* cur = Builder->GetInsertBlock();
            bool isDeadBlock = (cur != &fn->getEntryBlock()) && cur->hasNPredecessors(0);
            if (isDeadBlock)
                Builder->CreateUnreachable();
            else
                throw std::runtime_error("Non-void function '" + node.Name + "' has no return statement");
        }
    }

    auto verificationFailed = llvm::verifyFunction(*fn);

    if (verificationFailed) {
        fn->print(llvm::errs());
        throw std::runtime_error("Function verification failed for '" + node.Name + "'");
    }

    CurrentFunction = nullptr;
}

void CodeGenVisitor::visit(IfSttmt& node) {
    node.Cond->accept(*this);
    llvm::Value* cond = toBool(LastValue);

    auto* fn      = Builder->GetInsertBlock()->getParent();
    auto* thenBB  = llvm::BasicBlock::Create(Context, "then",   fn);
    auto* elseBB  = llvm::BasicBlock::Create(Context, "else");
    auto* mergeBB = llvm::BasicBlock::Create(Context, "ifcont");

    Builder->CreateCondBr(cond, thenBB, node.Else ? elseBB : mergeBB);

    // then
    Builder->SetInsertPoint(thenBB);
    if (node.Then) node.Then->accept(*this);
    if (!Builder->GetInsertBlock()->getTerminator())
        Builder->CreateBr(mergeBB);

    // else
    if (node.Else) {
        fn->insert(fn->end(), elseBB);
        Builder->SetInsertPoint(elseBB);
        node.Else->accept(*this);
        if (!Builder->GetInsertBlock()->getTerminator())
            Builder->CreateBr(mergeBB);
    }

    fn->insert(fn->end(), mergeBB);
    Builder->SetInsertPoint(mergeBB);
    LastValue = nullptr;
}

void CodeGenVisitor::visit(WhileSttmt& node) {
    auto* fn     = Builder->GetInsertBlock()->getParent();
    auto* condBB = llvm::BasicBlock::Create(Context, "while.cond", fn);
    auto* bodyBB = llvm::BasicBlock::Create(Context, "while.body", fn);
    auto* exitBB = llvm::BasicBlock::Create(Context, "while.exit", fn);

    auto* savedExit = LoopExit;
    auto* savedCont = LoopContinue;
    LoopExit = exitBB; LoopContinue = condBB;

    Builder->CreateBr(condBB);
    Builder->SetInsertPoint(condBB);
    node.Cond->accept(*this);
    Builder->CreateCondBr(toBool(LastValue), bodyBB, exitBB);

    Builder->SetInsertPoint(bodyBB);
    if (node.Do) node.Do->accept(*this);
    if (!Builder->GetInsertBlock()->getTerminator())
        Builder->CreateBr(condBB);

    Builder->SetInsertPoint(exitBB);
    LoopExit = savedExit; LoopContinue = savedCont;
    LastValue = nullptr;
}

void CodeGenVisitor::visit(JumpSttmt& node) {
    if (node.Name == "break" && LoopExit)
        Builder->CreateBr(LoopExit);
    else if (node.Name == "continue" && LoopContinue)
        Builder->CreateBr(LoopContinue);
}

void CodeGenVisitor::visit(ReturnSttmt& node) {
    if (node.ReturnValue) {
        node.ReturnValue->accept(*this);
        // Coerce to the function's declared return type (e.g. nter -> num widening)
        llvm::Type* retTy = Builder->GetInsertBlock()->getParent()->getReturnType();
        if (LastValue && LastValue->getType() != retTy)
            LastValue = coerce(LastValue, retTy);
        Builder->CreateRet(LastValue);
    } else {
        Builder->CreateRetVoid();
    }
    LastValue = nullptr;
}

void CodeGenVisitor::visit(FuncCallArgs& node) {
    // Handled inside FunCallExpr
}

void CodeGenVisitor::visit(FunCallExpr& node) {
    auto* fn = Mod->getFunction(node.Name);
    if (!fn) { LastValue = nullptr; return; }

    std::vector<llvm::Value*> callArgs;
    if (node.Args) {
        size_t i = 0;
        for (auto& arg : node.Args->Args) {
            arg->accept(*this);
            if (LastValue) {
                if (i < fn->arg_size()) {
                    // Coerce argument to the declared parameter type when they differ
                    llvm::Type* paramTy = (fn->arg_begin() + i)->getType();
                    if (LastValue->getType() != paramTy)
                        LastValue = coerce(LastValue, paramTy);
                }
                callArgs.push_back(LastValue);
                ++i;
            }
        }
    }

    LastValue = Builder->CreateCall(fn, callArgs);
}

void CodeGenVisitor::visit(BinExpr& node) {
    node.LHS->accept(*this);
    llvm::Value* lhs = LastValue;
    node.RHS->accept(*this);
    llvm::Value* rhs = LastValue;
    if (!lhs || !rhs) { LastValue = nullptr; return; }

    // Promote both sides to a common type: if either is double, use double;
    // otherwise keep integer arithmetic.
    auto* doubleTy = llvm::Type::getDoubleTy(Context);
    bool isFloat = lhs->getType()->isDoubleTy() || rhs->getType()->isDoubleTy();
    if (isFloat) { lhs = coerce(lhs, doubleTy); rhs = coerce(rhs, doubleTy); }

    const auto& op = node.Op;
    if      (op == "+")  LastValue = isFloat ? Builder->CreateFAdd(lhs, rhs) : Builder->CreateAdd(lhs, rhs);
    else if (op == "-")  LastValue = isFloat ? Builder->CreateFSub(lhs, rhs) : Builder->CreateSub(lhs, rhs);
    else if (op == "*")  LastValue = isFloat ? Builder->CreateFMul(lhs, rhs) : Builder->CreateMul(lhs, rhs);
    else if (op == "/")  LastValue = isFloat ? Builder->CreateFDiv(lhs, rhs) : Builder->CreateSDiv(lhs, rhs);
    else if (op == "%")  LastValue = isFloat ? Builder->CreateFRem(lhs, rhs) : Builder->CreateSRem(lhs, rhs);
    else if (op == "<")  LastValue = isFloat ? Builder->CreateFCmpOLT(lhs, rhs) : Builder->CreateICmpSLT(lhs, rhs);
    else if (op == ">")  LastValue = isFloat ? Builder->CreateFCmpOGT(lhs, rhs) : Builder->CreateICmpSGT(lhs, rhs);
    else if (op == "<=") LastValue = isFloat ? Builder->CreateFCmpOLE(lhs, rhs) : Builder->CreateICmpSLE(lhs, rhs);
    else if (op == ">=") LastValue = isFloat ? Builder->CreateFCmpOGE(lhs, rhs) : Builder->CreateICmpSGE(lhs, rhs);
    else if (op == "==") LastValue = isFloat ? Builder->CreateFCmpOEQ(lhs, rhs) : Builder->CreateICmpEQ(lhs, rhs);
    else if (op == "!=") LastValue = isFloat ? Builder->CreateFCmpONE(lhs, rhs) : Builder->CreateICmpNE(lhs, rhs);
    else if (op == "&&") LastValue = Builder->CreateAnd(toBool(lhs), toBool(rhs));
    else if (op == "||") LastValue = Builder->CreateOr(toBool(lhs), toBool(rhs));
    else LastValue = nullptr;
}

void CodeGenVisitor::visit(LiteralExpr& node) {
    const auto& t = node.Type;
    const auto& v = node.Value;

    if (t == "num") {
        LastValue = llvm::ConstantFP::get(Context, llvm::APFloat(std::stod(v)));
    } else if (t == "nter") {
        LastValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context),
                                           std::stoll(v), /*isSigned=*/true);
    } else if (t == "bool") {
        LastValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context),
                                           std::stoi(v) != 0);
    } else if (t == "char*") {
        std::string s = v;
        if (s.size() >= 2 && (s.front() == '"' || s.front() == '\'') && s.back() == s.front())
            s = s.substr(1, s.size() - 2);
        LastValue = Builder->CreateGlobalStringPtr(processEscapes(s));
    } else {
        LastValue = nullptr;
    }
}

void CodeGenVisitor::visit(ExprSttmt& node) {
    if (node.Expression) node.Expression->accept(*this);
    LastValue = nullptr;
}

void CodeGenVisitor::visit(IdentExpr& node) {
    auto* alloca = lookupVar(node.Name);
    if (alloca) {
        LastValue = Builder->CreateLoad(alloca->getAllocatedType(), alloca, node.Name);
        return;
    }
    auto* gv = lookupGlobal(node.Name);
    if (gv) {
        LastValue = Builder->CreateLoad(gv->getValueType(), gv, node.Name);
        return;
    }
    LastValue = nullptr;
}

void CodeGenVisitor::visit(ParExpr& node) {
    if (node.Content) node.Content->accept(*this);
}

void CodeGenVisitor::visit(AssignExpr& node) {
    if (node.Assigned) node.Assigned->accept(*this);
    llvm::Value* val = LastValue;

    auto* ident = dynamic_cast<IdentExpr*>(node.Assignee.get());
    auto* arrayAccess = dynamic_cast<ArrayAccessExpr*>(node.Assignee.get());
    if (!val || (!ident && !arrayAccess)) { LastValue = nullptr; return; }

    llvm::Value* destPtr = nullptr;
    llvm::Type* destTy = nullptr;

    if (ident) {
        if (auto* alloca = lookupVar(ident->Name)) {
            destPtr = alloca;
            destTy = alloca->getAllocatedType();
        } else if (auto* gv = lookupGlobal(ident->Name)) {
            destPtr = gv;
            destTy = gv->getValueType();
        }
    } else if (arrayAccess) {
        auto* storage = getArrayStorage(arrayAccess->Name);
        if (storage) {
            if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(storage)) destTy = alloca->getAllocatedType();
            else if (auto* gv = llvm::dyn_cast<llvm::GlobalVariable>(storage)) destTy = gv->getValueType();
            if (destTy && destTy->isArrayTy()) {
                if (arrayAccess->Index) arrayAccess->Index->accept(*this);
                if (!LastValue) { LastValue = nullptr; return; }

                auto* i64Ty = llvm::Type::getInt64Ty(Context);
                llvm::Value* idx = LastValue->getType() == i64Ty ? LastValue : coerce(LastValue, i64Ty);
                auto* sizeConst = llvm::ConstantInt::get(i64Ty, destTy->getArrayNumElements());
                auto* lineConst = llvm::ConstantInt::get(llvm::Type::getInt32Ty(Context), arrayAccess->LineNum);
                Builder->CreateCall(getOrDeclareKriolCheckBounds(), {idx, sizeConst, lineConst});
                destPtr = createArrayElementPtr(storage, destTy, idx);
                destTy = destTy->getArrayElementType();
            }
        }
    }

    if (!destPtr || !destTy) { LastValue = nullptr; return; }

    if (node.AssignOp != "=") {
        llvm::Value* cur = Builder->CreateLoad(destTy, destPtr);
        llvm::Value* rhs = coerce(val, destTy);
        bool isFloat = destTy->isDoubleTy();
        if      (node.AssignOp == "+=") val = isFloat ? Builder->CreateFAdd(cur, rhs) : Builder->CreateAdd(cur, rhs);
        else if (node.AssignOp == "-=") val = isFloat ? Builder->CreateFSub(cur, rhs) : Builder->CreateSub(cur, rhs);
        else if (node.AssignOp == "*=") val = isFloat ? Builder->CreateFMul(cur, rhs) : Builder->CreateMul(cur, rhs);
        else if (node.AssignOp == "/=") val = isFloat ? Builder->CreateFDiv(cur, rhs) : Builder->CreateSDiv(cur, rhs);
    } else {
        val = coerce(val, destTy);
    }
    Builder->CreateStore(val, destPtr);
    LastValue = val;
    return;
}

void CodeGenVisitor::visit(ForSttmt& node) {
    auto* fn      = Builder->GetInsertBlock()->getParent();
    auto* condBB  = llvm::BasicBlock::Create(Context, "for.cond",  fn);
    auto* bodyBB  = llvm::BasicBlock::Create(Context, "for.body",  fn);
    auto* afterBB = llvm::BasicBlock::Create(Context, "for.after", fn);
    auto* exitBB  = llvm::BasicBlock::Create(Context, "for.exit",  fn);

    auto* savedExit = LoopExit;
    auto* savedCont = LoopContinue;
    LoopExit = exitBB; LoopContinue = afterBB;

    if (node.Start) node.Start->accept(*this);
    Builder->CreateBr(condBB);

    Builder->SetInsertPoint(condBB);
    if (node.Cond) {
        node.Cond->accept(*this);
        Builder->CreateCondBr(toBool(LastValue), bodyBB, exitBB);
    } else {
        Builder->CreateBr(bodyBB);
    }

    Builder->SetInsertPoint(bodyBB);
    if (node.Then) node.Then->accept(*this);
    if (!Builder->GetInsertBlock()->getTerminator())
        Builder->CreateBr(afterBB);

    Builder->SetInsertPoint(afterBB);
    if (node.After) node.After->accept(*this);
    Builder->CreateBr(condBB);

    Builder->SetInsertPoint(exitBB);
    LoopExit = savedExit; LoopContinue = savedCont;
    LastValue = nullptr;
}

// Declare (or retrieve) one of the __kriol_print(n)_TYPE runtime functions.
static llvm::Function* getOrDeclareKriolPrint(llvm::Module& Mod, llvm::LLVMContext& Context,
        const std::string& suffix, llvm::Type* argTy, bool newline)
{
    std::string name = newline ? "__kriol_println_" + suffix : "__kriol_print_"  + suffix;
    if (auto* fn = Mod.getFunction(name)) return fn;
    auto* voidTy = llvm::Type::getVoidTy(Context);
    auto* ftype  = llvm::FunctionType::get(voidTy, {argTy}, false);
    return llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, name, Mod);
}

// Declare putchar if not already present.
static llvm::Function* getOrDeclarePutchar(llvm::Module& Mod, llvm::LLVMContext& Context)
{
    if (auto* fn = Mod.getFunction("putchar")) return fn;
    auto* ftype = llvm::FunctionType::get(llvm::Type::getInt32Ty(Context),
        {llvm::Type::getInt32Ty(Context)}, false);
    return llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "putchar", Mod);
}

// Declare (or retrieve) __kriol_bool_to_str: const char* __kriol_bool_to_str(int)
static llvm::Function* getOrDeclareKriolBoolToStr(llvm::Module& Mod, llvm::LLVMContext& Context)
{
    if (auto* fn = Mod.getFunction("__kriol_bool_to_str")) return fn;
    auto* ptrTy = llvm::PointerType::getUnqual(Context);
    auto* i32Ty = llvm::Type::getInt32Ty(Context);
    auto* ftype = llvm::FunctionType::get(ptrTy, {i32Ty}, false);
    return llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "__kriol_bool_to_str", Mod);
}

// Declare (or retrieve) __kriol_format -> char* __kriol_format(const char* fmt, ...)
static llvm::Function* getOrDeclareKriolFormat(llvm::Module& Mod, llvm::LLVMContext& Context)
{
    if (auto* fn = Mod.getFunction("__kriol_format")) return fn;
    auto* ptrTy  = llvm::PointerType::getUnqual(Context);
    auto* ftype  = llvm::FunctionType::get(ptrTy, {ptrTy}, /*isVarArg=*/true);
    return llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "__kriol_format", Mod);
}

static llvm::Type* getStorageValueType(llvm::Value* storage) {
    if (auto* alloca = llvm::dyn_cast<llvm::AllocaInst>(storage)) return alloca->getAllocatedType();
    if (auto* gv = llvm::dyn_cast<llvm::GlobalVariable>(storage)) return gv->getValueType();
    return nullptr;
}

void CodeGenVisitor::appendArrayFormatParts(llvm::Value* storage,
                                            llvm::ArrayType* arrayTy,
                                            std::string& outFmt,
                                            std::vector<llvm::Value*>& outArgs) {
    outFmt += "[";
    llvm::Type* elemTy    = arrayTy->getArrayElementType();
    uint64_t    elemCount = arrayTy->getNumElements();

    for (uint64_t i = 0; i < elemCount; ++i) {
        if (i > 0) outFmt += ", ";

        auto* idx      = llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context), i);
        llvm::Value* elemPtr = createArrayElementPtr(storage, arrayTy, idx);

        if (auto* nestedArrayTy = llvm::dyn_cast<llvm::ArrayType>(elemTy)) {
            appendArrayFormatParts(elemPtr, nestedArrayTy, outFmt, outArgs);
            continue;
        }

        llvm::Value* elem      = Builder->CreateLoad(elemTy, elemPtr, "fstr_array_elem");
        std::string  elemType  = llvmTypeToKriol(elemTy);
        if (elemType == "bool") {
            auto* i32Ty = llvm::Type::getInt32Ty(Context);
            llvm::Value* ext = elem->getType()->isIntegerTy(1)
                ? Builder->CreateZExt(elem, i32Ty)
                : elem;
            elem = Builder->CreateCall(getOrDeclareKriolBoolToStr(*Mod, Context), {ext}, "bool_str");
            outFmt += "%s";
        } else {
            outFmt += fmtSpec(elemType);
        }
        outArgs.push_back(elem);
    }

    outFmt += "]";
}

void CodeGenVisitor::visit(FStringExpr& node) {
    std::string fmtStr;
    std::vector<llvm::Value*> callArgs;

    for (auto& seg : node.Parts) {
        if (!seg.expr) { // Literal text
            std::string text = processEscapes(seg.text);
            for (char c : text) {
                if (c == '%') fmtStr += '%';
                fmtStr += c;
            }
        } else { // Interpolated expression
            auto parsedType = parseType(seg.expr->ResolvedType);
            if (parsedType && parsedType->isArray()) {
                auto* ident = unwrapIdentExpr(seg.expr.get());
                if (!ident)
                    throw std::runtime_error("array interpolation currently supports array variables only");

                llvm::Value* storage = getArrayStorage(ident->Name);
                llvm::Type* storageTy = getStorageValueType(storage);
                auto* arrayTy = storageTy ? llvm::dyn_cast<llvm::ArrayType>(storageTy) : nullptr;
                if (!storage || !arrayTy)
                    throw std::runtime_error("cannot interpolate array variable '" + ident->Name + "'");

                appendArrayFormatParts(storage, arrayTy, fmtStr, callArgs);
                continue;
            }

            seg.expr->accept(*this);
            llvm::Value* val = LastValue;
            if (!val) continue;

            std::string kriolType = seg.expr->ResolvedType;
            if (kriolType.empty()) kriolType = llvmTypeToKriol(val->getType());

            if (kriolType == "bool") {
                auto* i32Ty = llvm::Type::getInt32Ty(Context);
                llvm::Value* ext = val->getType()->isIntegerTy(1)
                    ? Builder->CreateZExt(val, i32Ty) : val;
                val = Builder->CreateCall(
                    getOrDeclareKriolBoolToStr(*Mod, Context), {ext}, "bool_str");
                fmtStr += "%s";
            } else {
                fmtStr += fmtSpec(kriolType);
            }
            callArgs.push_back(val);
        }
    }

    auto* fmtGstr = Builder->CreateGlobalStringPtr(fmtStr, "fstr_fmt");
    callArgs.insert(callArgs.begin(), fmtGstr);
    auto* formatFn = getOrDeclareKriolFormat(*Mod, Context);
    LastValue = Builder->CreateCall(formatFn, callArgs, "fstr");
}

void CodeGenVisitor::visit(MostraFunCallExpr& node) {
    auto* ptrTy    = llvm::PointerType::getUnqual(Context);
    auto* i64Ty    = llvm::Type::getInt64Ty(Context);
    auto* doubleTy = llvm::Type::getDoubleTy(Context);
    auto* i32Ty    = llvm::Type::getInt32Ty(Context);
    auto* putcharFn = getOrDeclarePutchar(*Mod, Context);

    auto emitPrintValue = [&](llvm::Value* val, const std::string& resolvedType, bool newline) {
        if (resolvedType == "nter") {
            if (val->getType() != i64Ty) val = coerce(val, i64Ty);
            Builder->CreateCall(getOrDeclareKriolPrint(*Mod, Context, "nter", i64Ty, newline), {val});
        } else if (resolvedType == "num") {
            if (val->getType() != doubleTy) val = coerce(val, doubleTy);
            Builder->CreateCall(getOrDeclareKriolPrint(*Mod, Context, "num", doubleTy, newline), {val});
        } else if (resolvedType == "bool") {
            llvm::Value* ext = val->getType()->isIntegerTy(1)
                ? Builder->CreateZExt(val, i32Ty, "bool_ext")
                : Builder->CreateTrunc(val, i32Ty, "bool_ext");
            Builder->CreateCall(getOrDeclareKriolPrint(*Mod, Context, "bool", i32Ty, newline), {ext});
        } else {
            if (!val->getType()->isPointerTy()) val = Builder->CreateIntToPtr(val, ptrTy);
            Builder->CreateCall(getOrDeclareKriolPrint(*Mod, Context, "textu", ptrTy, newline), {val});
        }
    };

    auto printChar = [&](char c) {
        Builder->CreateCall(putcharFn, {llvm::ConstantInt::get(i32Ty, c)});
    };

    auto emitPrintArray = [&](auto&& self, llvm::Value* storage, llvm::ArrayType* arrayTy) -> void {
        printChar('[');
        llvm::Type* elemTy = arrayTy->getArrayElementType();
        uint64_t elemCount = arrayTy->getNumElements();

        for (uint64_t i = 0; i < elemCount; ++i) {
            if (i > 0) {
                printChar(',');
                printChar(' ');
            }

            auto* idx = llvm::ConstantInt::get(i64Ty, i);
            llvm::Value* elemPtr = createArrayElementPtr(storage, arrayTy, idx);

            if (auto* nestedArrayTy = llvm::dyn_cast<llvm::ArrayType>(elemTy)) {
                self(self, elemPtr, nestedArrayTy);
                continue;
            }

            llvm::Value* elem = Builder->CreateLoad(elemTy, elemPtr, "array.elem");
            emitPrintValue(elem, llvmTypeToKriol(elemTy), /*newline=*/false);
        }

        printChar(']');
    };

    if (!node.Args || node.Args->Args.empty()) {
        if (node.AddNewline)
            Builder->CreateCall(putcharFn,
                                {llvm::ConstantInt::get(i32Ty, '\n')});
        LastValue = nullptr;
        return;
    }

    auto& args    = node.Args->Args;
    size_t lastIdx = args.size() - 1;

    for (size_t i = 0; i < args.size(); ++i) {
        bool addNlHere = (i == lastIdx) && node.AddNewline;
        auto& arg = args[i];

        auto parsedType = parseType(arg->ResolvedType);
        if (parsedType && parsedType->isArray()) {
            auto* ident = unwrapIdentExpr(arg.get());
            if (!ident)
                throw std::runtime_error("array printing currently supports array variables only");

            llvm::Value* storage = getArrayStorage(ident->Name);
            llvm::Type* storageTy = getStorageValueType(storage);
            auto* arrayTy = storageTy ? llvm::dyn_cast<llvm::ArrayType>(storageTy) : nullptr;
            if (!storage || !arrayTy)
                throw std::runtime_error("cannot print array variable '" + ident->Name + "'");

            emitPrintArray(emitPrintArray, storage, arrayTy);
            if (addNlHere) printChar('\n');
            continue;
        }

        arg->accept(*this);
        llvm::Value* v = LastValue;
        if (v) emitPrintValue(v, arg->ResolvedType, addNlHere);
    }

    LastValue = nullptr;
}

void CodeGenVisitor::visit(ImportSttmt& node) {
    // TODO: Implement module imports
}

void CodeGenVisitor::visit(UnaryExpr& node) {
    if (!node.Operand) { LastValue = nullptr; return; }
    node.Operand->accept(*this);

    llvm::Value* v = LastValue;
    if (!v) { LastValue = nullptr; return; }

    if (node.Op == "!") {
        LastValue = Builder->CreateNot(toBool(v), "nottmp");
    } else { // "-"
        if (v->getType()->isDoubleTy())
            LastValue = Builder->CreateFNeg(v, "negtmp");
        else
            LastValue = Builder->CreateNeg(v, "negtmp");
    }
}

// Declare (or retrieve) libc exit
static llvm::Function* getOrDeclareExit(llvm::Module& Mod, llvm::LLVMContext& Context)
{
    if (auto* fn = Mod.getFunction("exit")) return fn;
    auto* voidTy = llvm::Type::getVoidTy(Context);
    auto* i32Ty  = llvm::Type::getInt32Ty(Context);
    auto* ftype  = llvm::FunctionType::get(voidTy, {i32Ty}, false);
    auto* fn = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "exit", Mod);
    fn->addFnAttr(llvm::Attribute::NoReturn);
    return fn;
}

// Declare (or retrieve) __kriol_konfirma -> void __kriol_konfirma(i32 cond, i32 line)
static llvm::Function* getOrDeclareKriolKonfirma(llvm::Module& Mod, llvm::LLVMContext& Context)
{
    if (auto* fn = Mod.getFunction("__kriol_konfirma")) return fn;
    auto* voidTy = llvm::Type::getVoidTy(Context);
    auto* i32Ty  = llvm::Type::getInt32Ty(Context);
    auto* ftype  = llvm::FunctionType::get(voidTy, {i32Ty, i32Ty}, false);
    return llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, "__kriol_konfirma", Mod);
}

void CodeGenVisitor::visit(SaiSttmt& node) {
    auto* i32Ty = llvm::Type::getInt32Ty(Context);
    // Default to exit code 0 if no expression is provided.
    llvm::Value* code = llvm::ConstantInt::get(i32Ty, 0);
    if (node.Code) {
        node.Code->accept(*this);
        if (LastValue) code = coerce(LastValue, i32Ty);
    }
    Builder->CreateCall(getOrDeclareExit(*Mod, Context), {code});
    Builder->CreateUnreachable();
    LastValue = nullptr;
}

void CodeGenVisitor::visit(KonfirmaSttmt& node) {
    auto* i32Ty = llvm::Type::getInt32Ty(Context);
    // Default to true (1) if no condition is provided.
    llvm::Value* cond = llvm::ConstantInt::get(i32Ty, 1);
    if (node.Cond) {
        node.Cond->accept(*this);
        if (LastValue) {
            llvm::Value* b = toBool(LastValue);
            cond = Builder->CreateZExt(b, i32Ty, "konfirma_cond");
        }
    }
    llvm::Value* line = llvm::ConstantInt::get(i32Ty, node.LineNum);
    Builder->CreateCall(getOrDeclareKriolKonfirma(*Mod, Context), {cond, line});
    LastValue = nullptr;
}
