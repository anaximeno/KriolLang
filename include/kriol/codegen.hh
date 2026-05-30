#ifndef _KRIOL_CODEGEN_HEADER
#define _KRIOL_CODEGEN_HEADER

#include "ast.hh"

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/BasicBlock.h>

namespace kriol {
namespace ast {

    class CodeGenVisitor : public Visitor {
    private:
        llvm::LLVMContext Context;
        std::unique_ptr<llvm::Module> Mod;
        std::unique_ptr<llvm::IRBuilder<>> Builder;

        // Result register for expression visits
        llvm::Value* LastValue = nullptr;

        // Currently-emitting function
        llvm::Function* CurrentFunction = nullptr;

        // Loop exit / continue targets (for break / continue)
        llvm::BasicBlock* LoopExit     = nullptr;
        llvm::BasicBlock* LoopContinue = nullptr;

        // Scope stack: variable name -> AllocaInst*
        std::vector<std::unordered_map<std::string, llvm::AllocaInst*>> Scopes;

        // Module-scope globals: variable name -> GlobalVariable*
        std::unordered_map<std::string, llvm::GlobalVariable*> GlobalVars;

        // Non-constant global initializers deferred until inisiu's preamble
        struct DeferredGlobalInit {
            llvm::GlobalVariable*  Var;
            // non-owning; AST outlives codegen
            kriol::ast::Expr*      InitExpr;
        };
        std::vector<DeferredGlobalInit> DeferredGlobalInits;

        // Type table: variable name -> Kriol type string
        std::unordered_map<std::string, std::string> TypeTable;

        llvm::Type*          mapType(const std::string& kriolType);
        llvm::AllocaInst*    createEntryAlloca(llvm::Function* fn,
                                               const std::string& name,
                                               llvm::Type* ty);
        llvm::AllocaInst*    lookupVar(const std::string& name);
        llvm::GlobalVariable* lookupGlobal(const std::string& name);

        void pushScope() { Scopes.push_back({}); }
        void popScope()  { if (!Scopes.empty()) Scopes.pop_back(); }
        void declareVar(const std::string& name, llvm::AllocaInst* a) {
            if (!Scopes.empty()) Scopes.back()[name] = a;
        }

        llvm::Function* getOrDeclarePrintf();

        // Central scalar coercion table: convert v to targetTy.
        // Supported pairs: nter->num (SIToFP), bool->nter (ZExt),
        // bool->num (UIToFP), num->nter (FPToSI). Identity is a no-op.
        // Throws for unsupported or pointer conversions.
        llvm::Value* coerce(llvm::Value* v, llvm::Type* targetTy);

        // Coerce value to i1 for use as a branch condition
        llvm::Value* toBool(llvm::Value* v);

    public:
        explicit CodeGenVisitor(const std::string& moduleName);

        /// Serialise the module as LLVM IR text.
        std::string emitIR();

        /// Compile the module to a native executable at outputPath.
        /// argv0 is used to locate the runtime object relative to the executable.
        void emitNative(const std::string& outputPath, const char* argv0 = nullptr);

        void visit(VarDeclSttmt&      node) override;
        void visit(BlockSttmt&        node) override;
        void visit(FuncArgs&          node) override;
        void visit(FuncDeclSttmt&     node) override;
        void visit(IfSttmt&           node) override;
        void visit(WhileSttmt&        node) override;
        void visit(JumpSttmt&         node) override;
        void visit(ReturnSttmt&       node) override;
        void visit(FuncCallArgs&      node) override;
        void visit(FunCallExpr&       node) override;
        void visit(BinExpr&           node) override;
        void visit(LiteralExpr&       node) override;
        void visit(ExprSttmt&         node) override;
        void visit(IdentExpr&         node) override;
        void visit(ParExpr&           node) override;
        void visit(AssignExpr&        node) override;
        void visit(ForSttmt&          node) override;
        void visit(MostraFunCallExpr& node) override;
        void visit(ImportSttmt&       node) override;
        void visit(FStringExpr&       node) override;
        void visit(UnaryExpr&         node) override;
        void visit(SaiSttmt&          node) override;
        void visit(KonfirmaSttmt&     node) override;
    };

} // namespace ast
} // namespace kriol

#endif // _KRIOL_CODEGEN_HEADER
