#include "../../include/kriol/codegen.hh"

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

#include <llvm/Support/Program.h>
#include <llvm/Support/Path.h>

#include <stdexcept>
#include <cstdlib>
#include <cstdio>

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

using namespace kriol::ast;

CodeGenVisitor::CodeGenVisitor(const std::string& moduleName)
    : Mod(std::make_unique<llvm::Module>(moduleName, Context)),
      Builder(std::make_unique<llvm::IRBuilder<>>(Context))
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
}

llvm::Type* CodeGenVisitor::mapType(const std::string& t) {
    if (t == "num")   return llvm::Type::getDoubleTy(Context);
    if (t == "nter")  return llvm::Type::getInt64Ty(Context);
    if (t == "bool")  return llvm::Type::getInt1Ty(Context);
    if (t == "textu") return llvm::PointerType::getUnqual(Context);
    if (t == "vaziu") return llvm::Type::getVoidTy(Context);
    return llvm::Type::getDoubleTy(Context); // default
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

void CodeGenVisitor::emitNative(const std::string& outputPath, const char* argv0) {
    auto triple = llvm::sys::getDefaultTargetTriple();
    Mod->setTargetTriple(triple);

    std::string err;
    auto* target = llvm::TargetRegistry::lookupTarget(triple, err);
    if (!target)
        throw std::runtime_error("LLVM target lookup failed: " + err);

    llvm::TargetOptions opt;
    auto* tm = target->createTargetMachine(
        triple, "generic", "", opt, llvm::Reloc::PIC_);
    Mod->setDataLayout(tm->createDataLayout());

    std::string objPath = outputPath + ".o";
    std::error_code ec;
    llvm::raw_fd_ostream dest(objPath, ec, llvm::sys::fs::OF_None);
    if (ec)
        throw std::runtime_error("Cannot open output file: " + ec.message());

    llvm::legacy::PassManager pm;
    if (tm->addPassesToEmitFile(pm, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
        throw std::runtime_error("TargetMachine cannot emit an object file");

    pm.run(*Mod);
    dest.flush();

    auto ccPath = llvm::sys::findProgramByName("clang");

    if (!ccPath)
        throw std::runtime_error("Cannot find 'clang': " + ccPath.getError().message());

#ifdef KRIOL_RUNTIME_OBJ
    // Resolve the runtime object relative to the running executable.
    static int anchor;
    std::string exePath = llvm::sys::fs::getMainExecutable(argv0, (void*)&anchor);
    llvm::SmallString<256> runtimePath(llvm::sys::path::parent_path(exePath));
    llvm::sys::path::append(runtimePath, KRIOL_RUNTIME_OBJ);
    std::string runtimeObj(runtimePath);
#endif

    std::vector<llvm::StringRef> linkArgs = {
        *ccPath,
        objPath,
#ifdef KRIOL_RUNTIME_OBJ
        runtimeObj,
#endif
        "-o",
        outputPath,
        "-lm",
        "-lgc"
    };

    std::string linkErr;
    bool execFailed = false;
    int ret = llvm::sys::ExecuteAndWait(
        *ccPath,
        linkArgs,
        /*Env=*/std::nullopt,
        /*Redirects=*/{},
        /*SecondsToWait=*/0,
        /*MemoryLimit=*/0,
        &linkErr,
        &execFailed
    );

    if (execFailed || ret != 0)
        throw std::runtime_error("Linking failed" + (linkErr.empty() ? "" : ": " + linkErr));

    std::remove(objPath.c_str());
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

    if (node.Value) {
        node.Value->accept(*this);
        if (LastValue) {
            LastValue = coerce(LastValue, ty);
            Builder->CreateStore(LastValue, alloca);
        }
    }
    LastValue = nullptr;
}

void CodeGenVisitor::forwardDeclareFunc(ast::FuncDeclSttmt& node) {
    bool isMain   = (node.Name == "inisiu");
    std::string name = isMain ? "main" : node.Name;

    // already declared
    if (Mod->getFunction(name)) return;

    std::vector<llvm::Type*> paramTypes;
    if (node.Args)
        for (auto& arg : node.Args->Args)
            paramTypes.push_back(mapType(arg->Type));

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
    }

    pushScope();
    for (auto& s : node.SttmtList)
        if (s) s->accept(*this);
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
    std::string name = isMain ? "main" : node.Name;

    std::vector<llvm::Type*> paramTypes;
    if (node.Args)
        for (auto& arg : node.Args->Args)
            paramTypes.push_back(mapType(arg->Type));

    llvm::Type* retTy = isMain
        ? llvm::Type::getInt32Ty(Context)
        : mapType(node.Type);

    // Get the function if it was already forward-declared, otherwise create it now.
    auto* fn = Mod->getFunction(name);

    if (!fn) {
        auto* ftype = llvm::FunctionType::get(retTy, paramTypes, false);
        fn = llvm::Function::Create(ftype, llvm::Function::ExternalLinkage, name, *Mod);
    }

    if (node.Args) {
        size_t i = 0;
        for (auto& llvmArg : fn->args())
            llvmArg.setName(node.Args->Args[i++]->Name);
    }

    auto* entry = llvm::BasicBlock::Create(Context, "entry", fn);
    Builder->SetInsertPoint(entry);
    CurrentFunction = fn;

    pushScope();
    if (node.Args) {
        size_t i = 0;
        for (auto& llvmArg : fn->args()) {
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
    if (!ident || !val) { LastValue = nullptr; return; }

    auto* alloca = lookupVar(ident->Name);
    if (alloca) {
        if (node.AssignOp != "=") {
            llvm::Value* cur = Builder->CreateLoad(alloca->getAllocatedType(), alloca);
            llvm::Type* ty = cur->getType();
            llvm::Value* rhs = coerce(val, ty);
            bool isFloat = ty->isDoubleTy();
            if      (node.AssignOp == "+=") val = isFloat ? Builder->CreateFAdd(cur, rhs) : Builder->CreateAdd(cur, rhs);
            else if (node.AssignOp == "-=") val = isFloat ? Builder->CreateFSub(cur, rhs) : Builder->CreateSub(cur, rhs);
            else if (node.AssignOp == "*=") val = isFloat ? Builder->CreateFMul(cur, rhs) : Builder->CreateMul(cur, rhs);
            else if (node.AssignOp == "/=") val = isFloat ? Builder->CreateFDiv(cur, rhs) : Builder->CreateSDiv(cur, rhs);
        } else {
            val = coerce(val, alloca->getAllocatedType());
        }
        Builder->CreateStore(val, alloca);
        LastValue = val;
        return;
    }

    auto* gv = lookupGlobal(ident->Name);
    if (gv) {
        if (node.AssignOp != "=") {
            llvm::Value* cur = Builder->CreateLoad(gv->getValueType(), gv);
            llvm::Type* ty = cur->getType();
            llvm::Value* rhs = coerce(val, ty);
            bool isFloat = ty->isDoubleTy();
            if      (node.AssignOp == "+=") val = isFloat ? Builder->CreateFAdd(cur, rhs) : Builder->CreateAdd(cur, rhs);
            else if (node.AssignOp == "-=") val = isFloat ? Builder->CreateFSub(cur, rhs) : Builder->CreateSub(cur, rhs);
            else if (node.AssignOp == "*=") val = isFloat ? Builder->CreateFMul(cur, rhs) : Builder->CreateMul(cur, rhs);
            else if (node.AssignOp == "/=") val = isFloat ? Builder->CreateFDiv(cur, rhs) : Builder->CreateSDiv(cur, rhs);
        } else {
            val = coerce(val, gv->getValueType());
        }
        Builder->CreateStore(val, gv);
    }
    LastValue = val;
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

    if (!node.Args || node.Args->Args.empty()) {
        if (node.AddNewline)
            Builder->CreateCall(getOrDeclarePutchar(*Mod, Context),
                                {llvm::ConstantInt::get(i32Ty, '\n')});
        LastValue = nullptr;
        return;
    }

    auto& args    = node.Args->Args;
    size_t lastIdx = args.size() - 1;

    for (size_t i = 0; i < args.size(); ++i) {
        bool addNlHere = (i == lastIdx) && node.AddNewline;
        auto& arg = args[i];

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
