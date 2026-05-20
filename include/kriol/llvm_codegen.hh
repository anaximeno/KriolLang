#ifndef _KRIOL_LLVM_CODEGEN_HEADER
#define _KRIOL_LLVM_CODEGEN_HEADER

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

    class LLVMCodeGenVisitor : public Visitor {
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

        // Type table: variable name -> Kriol type string
        std::unordered_map<std::string, std::string> TypeTable;

        llvm::Type*       mapType(const std::string& kriolType);
        llvm::AllocaInst* createEntryAlloca(llvm::Function* fn,
                                            const std::string& name,
                                            llvm::Type* ty);
        llvm::AllocaInst* lookupVar(const std::string& name);

        void pushScope() { Scopes.push_back({}); }
        void popScope()  { if (!Scopes.empty()) Scopes.pop_back(); }
        void declareVar(const std::string& name, llvm::AllocaInst* a) {
            if (!Scopes.empty()) Scopes.back()[name] = a;
        }

        llvm::Function* getOrDeclarePrintf();
        llvm::Value*    coerceToDouble(llvm::Value* v);

        // Coerce value to i1 for use as a branch condition
        llvm::Value* toBool(llvm::Value* v);

    public:
        explicit LLVMCodeGenVisitor(const std::string& moduleName);

        /// Serialise the module as LLVM IR text.
        std::string emitIR();

        /// Compile the module to a native executable at outputPath.
        void emitNative(const std::string& outputPath);

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
    };

} // namespace ast
} // namespace kriol

#endif // _KRIOL_LLVM_CODEGEN_HEADER
