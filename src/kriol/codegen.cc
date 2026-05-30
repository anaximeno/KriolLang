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

#include <stdexcept>
#include <cstdlib>
#include <cstdio>

// Interpret C-style backslash escapes in a raw string (without surrounding quotes).
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
                default:   out += '\\'; out += raw[i]; break;
            }
        } else {
            out += raw[i];
        }
    }
    return out;
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

void CodeGenVisitor::emitNative(const std::string& outputPath) {
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

    auto ccPath = llvm::sys::findProgramByName("cc");

    if (!ccPath)
        throw std::runtime_error("Cannot find 'cc': " + ccPath.getError().message());

    std::vector<llvm::StringRef> linkArgs = {
        *ccPath, objPath, "-o", outputPath, "-lm"
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
        if (LastValue) Builder->CreateStore(LastValue, alloca);
    }
    LastValue = nullptr;
}

void CodeGenVisitor::visit(BlockSttmt& node) {
    pushScope();
    for (auto& s : node.SttmtList)
        if (s) s->accept(*this);
    popScope();
}

void CodeGenVisitor::visit(FuncArgs& node) {
    // Handled inside FuncDeclSttmt
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

    auto* ftype = llvm::FunctionType::get(retTy, paramTypes, false);
    auto* fn    = llvm::Function::Create(
        ftype, llvm::Function::ExternalLinkage, name, *Mod);

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

    // Emit deferred global initializers at the top of inisiu (main)
    if (isMain && !DeferredGlobalInits.empty()) {
        for (auto& di : DeferredGlobalInits) {
            di.InitExpr->accept(*this);
            if (LastValue) Builder->CreateStore(LastValue, di.Var);
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

    if (t == "float" || t == "num") {
        LastValue = llvm::ConstantFP::get(Context, llvm::APFloat(std::stod(v)));
    } else if (t == "int" || t == "nter") {
        LastValue = llvm::ConstantInt::get(llvm::Type::getInt64Ty(Context),
                                           std::stoll(v), /*isSigned=*/true);
    } else if (t == "unsigned short" || t == "bool") {
        LastValue = llvm::ConstantInt::get(llvm::Type::getInt1Ty(Context),
                                           std::stoi(v) != 0);
    } else if (t == "char*" || t == "textu") {
        std::string s = v;
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"')
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

void CodeGenVisitor::visit(MostraFunCallExpr& node) {
    if (!node.Args || node.Args->Args.empty()) return;
    auto& args = node.Args->Args;

    // If first arg is a string literal, pass through as-is (user format string)
    if (auto* first = dynamic_cast<LiteralExpr*>(args[0].get())) {
        if (first->Type == "char*") {
            std::vector<llvm::Value*> callArgs;
            for (auto& arg : args) {
                arg->accept(*this);
                if (LastValue) callArgs.push_back(LastValue);
            }
            Builder->CreateCall(getOrDeclarePrintf(), callArgs);
            LastValue = nullptr;
            return;
        }
    }

    // Auto-format: derive format string from Kriol types
    std::string fmt;
    for (auto& arg : args) {
        std::string spec = "%g"; // default: double
        if (auto* id = dynamic_cast<IdentExpr*>(arg.get())) {
            auto it = TypeTable.find(id->Name);
            if (it != TypeTable.end()) {
                if (it->second == "nter")  spec = "%lld";
                else if (it->second == "textu") spec = "%s";
                else if (it->second == "bool")  spec = "%d";
                else spec = "%g";
            }
        } else if (auto* lit = dynamic_cast<LiteralExpr*>(arg.get())) {
            if (lit->Type == "int" || lit->Type == "nter")        spec = "%lld";
            else if (lit->Type == "char*" || lit->Type == "textu") spec = "%s";
            else if (lit->Type == "bool")                          spec = "%d";
        }
        fmt += spec;
    }

    std::vector<llvm::Value*> callArgs;
    callArgs.push_back(Builder->CreateGlobalStringPtr(fmt));
    for (auto& arg : args) {
        arg->accept(*this);
        if (LastValue) callArgs.push_back(LastValue);
    }
    Builder->CreateCall(getOrDeclarePrintf(), callArgs);
    LastValue = nullptr;
}

void CodeGenVisitor::visit(ImportSttmt& node) {
    // C #include directives have no LLVM IR equivalent; skipped.
}
