#ifndef _KRIOL_CODEGEN_HEADER
#define _KRIOL_CODEGEN_HEADER

#include "ast.hh"
#include "cbinds.hh"
#include <iostream>
#include <ostream>

namespace kriol {
namespace ast {

    class CodeGenVisitor : public Visitor {
    private:
        std::ostream& os;

    public:
        CodeGenVisitor(std::ostream& os) : os(os) {}

        void visit(VarDeclSttmt& node) override;
        void visit(BlockSttmt& node) override;
        void visit(FuncArgs& node) override;
        void visit(FuncDeclSttmt& node) override;
        void visit(IfSttmt& node) override;
        void visit(WhileSttmt& node) override;
        void visit(JumpSttmt& node) override;
        void visit(ReturnSttmt& node) override;
        void visit(FuncCallArgs& node) override;
        void visit(FunCallExpr& node) override;
        void visit(BinExpr& node) override;
        void visit(LiteralExpr& node) override;
        void visit(ExprSttmt& node) override;
        void visit(IdentExpr& node) override;
        void visit(ParExpr& node) override;
        void visit(AssignExpr& node) override;
        void visit(ForSttmt& node) override;
        void visit(MostraFunCallExpr& node) override;
        void visit(ImportSttmt& node) override;
    };

}
}

#endif // _KRIOL_CODEGEN_HEADER
