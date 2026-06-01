#ifndef _KRIOL_AST_TRANSPILER_HEADER
#define _KRIOL_AST_TRANSPILER_HEADER

#include <string>
#include <vector>
#include <memory>
#include <utility>


namespace kriol {
namespace ast {

    // Forward declarations for Visitor
    class Expr;
    class VarDeclSttmt;
    class BlockSttmt;
    class FuncArgs;
    class FuncDeclSttmt;
    class IfSttmt;
    class WhileSttmt;
    class JumpSttmt;
    class ReturnSttmt;
    class FuncCallArgs;
    class FunCallExpr;
    class BinExpr;
    class LiteralExpr;
    class ExprSttmt;
    class IdentExpr;
    class ParExpr;
    class AssignExpr;
    class ForSttmt;
    class MostraFunCallExpr;
    class ImportSttmt;
    class ArrayAccessExpr;
    class ArrayLiteralExpr;
    class ArrayRepeatExpr;
    class FStringExpr;
    class UnaryExpr;
    class SaiSttmt;
    class KonfirmaSttmt;

    class Visitor {
    public:
        virtual ~Visitor() = default;
        virtual void visit(VarDeclSttmt& node) = 0;
        virtual void visit(BlockSttmt& node) = 0;
        virtual void visit(FuncArgs& node) = 0;
        virtual void visit(FuncDeclSttmt& node) = 0;
        virtual void visit(IfSttmt& node) = 0;
        virtual void visit(WhileSttmt& node) = 0;
        virtual void visit(JumpSttmt& node) = 0;
        virtual void visit(ReturnSttmt& node) = 0;
        virtual void visit(FuncCallArgs& node) = 0;
        virtual void visit(FunCallExpr& node) = 0;
        virtual void visit(BinExpr& node) = 0;
        virtual void visit(LiteralExpr& node) = 0;
        virtual void visit(ExprSttmt& node) = 0;
        virtual void visit(IdentExpr& node) = 0;
        virtual void visit(ParExpr& node) = 0;
        virtual void visit(AssignExpr& node) = 0;
        virtual void visit(ForSttmt& node) = 0;
        virtual void visit(MostraFunCallExpr& node) = 0;
        virtual void visit(ImportSttmt& node) = 0;
        virtual void visit(ArrayAccessExpr& node) = 0;
        virtual void visit(ArrayLiteralExpr& node) = 0;
        virtual void visit(ArrayRepeatExpr& node) = 0;
        virtual void visit(FStringExpr& node) = 0;
        virtual void visit(UnaryExpr& node) = 0;
        virtual void visit(SaiSttmt& node) = 0;
        virtual void visit(KonfirmaSttmt& node) = 0;
    };

    class Sttmt {
    public:
        int LineNum = 0;
        virtual ~Sttmt() = default;
        virtual void accept(Visitor& v) = 0;
    };

    class Expr : public Sttmt {
    public:
        std::string ResolvedType;
        virtual ~Expr() = default;
    };

    class VarDeclSttmt : public Sttmt {
    public:
        std::string Type;
        std::string Name;
        std::unique_ptr<Expr> Value;
        bool IsParam = false;
        bool IsArray = false;
        std::size_t ArraySize = 0;

        VarDeclSttmt(std::string Type, std::string Name, std::unique_ptr<Expr> Value)
            : Type(std::move(Type)), Name(std::move(Name)), Value(std::move(Value)) {}
        void accept(Visitor& v) override { v.visit(*this); }
        void SetType(std::string type) { Type = std::move(type); }
    };

    class BlockSttmt : public Sttmt {
    public:
        std::vector<std::unique_ptr<Sttmt>> SttmtList;
        bool BracketsOn = false;

        void AddSttmt(std::unique_ptr<Sttmt> sttmt) { SttmtList.push_back(std::move(sttmt)); }
        void UseBrackets() { BracketsOn = true; }
        void DontUseBrackets() { BracketsOn = false; }
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class FuncArgs {
    public:
        std::vector<std::unique_ptr<VarDeclSttmt>> Args;
        void AddArg(std::unique_ptr<VarDeclSttmt> Arg) { Args.push_back(std::move(Arg)); }
        void accept(Visitor& v) { v.visit(*this); }
    };

    class FuncDeclSttmt : public Sttmt {
    public:
        std::string Type;
        std::string Name;
        std::unique_ptr<FuncArgs> Args;
        std::unique_ptr<BlockSttmt> Body;

        FuncDeclSttmt(std::string Type, std::string Name, std::unique_ptr<FuncArgs> Args, std::unique_ptr<BlockSttmt> Body)
            : Type(std::move(Type)), Name(std::move(Name)), Args(std::move(Args)), Body(std::move(Body)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class IfSttmt : public Sttmt {
    public:
        std::unique_ptr<Expr> Cond;
        std::unique_ptr<BlockSttmt> Then;
        std::unique_ptr<BlockSttmt> Else;

        IfSttmt(std::unique_ptr<Expr> Cond, std::unique_ptr<BlockSttmt> Then, std::unique_ptr<BlockSttmt> Else)
            : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class WhileSttmt : public Sttmt {
    public:
        std::unique_ptr<Expr> Cond;
        std::unique_ptr<BlockSttmt> Do;

        WhileSttmt(std::unique_ptr<Expr> Cond, std::unique_ptr<BlockSttmt> Do)
            : Cond(std::move(Cond)), Do(std::move(Do)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class JumpSttmt : public Sttmt {
    public:
        std::string Name;

        JumpSttmt(std::string Name) : Name(std::move(Name)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class ReturnSttmt : public JumpSttmt {
    public:
        std::unique_ptr<Expr> ReturnValue;

        ReturnSttmt(std::unique_ptr<Expr> ReturnValue)
            : JumpSttmt("return"), ReturnValue(std::move(ReturnValue)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class FuncCallArgs {
    public:
        std::vector<std::unique_ptr<Expr>> Args;
        void AddArg(std::unique_ptr<Expr> Arg) { Args.push_back(std::move(Arg)); }
        void accept(Visitor& v) { v.visit(*this); }
    };

    class FunCallExpr : public Expr {
    public:
        std::string Name;
        std::unique_ptr<FuncCallArgs> Args;

        FunCallExpr(std::string Name, std::unique_ptr<FuncCallArgs> Args)
            : Name(std::move(Name)), Args(std::move(Args)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class MostraFunCallExpr : public FunCallExpr {
    public:
        bool AddNewline = false;

        MostraFunCallExpr(std::unique_ptr<FuncCallArgs> Args, bool addNewline = false)
            : FunCallExpr("printf", std::move(Args)), AddNewline(addNewline) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class BinExpr : public Expr {
    public:
        std::string Op;
        std::unique_ptr<Expr> LHS;
        std::unique_ptr<Expr> RHS;

        BinExpr(std::string Op, std::unique_ptr<Expr> LHS, std::unique_ptr<Expr> RHS)
            : Op(std::move(Op)), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class LiteralExpr : public Expr {
    public:
        std::string Type;
        std::string Value;

        LiteralExpr(std::string Type, std::string Value)
            : Type(std::move(Type)), Value(std::move(Value)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class ExprSttmt : public Expr {
    public:
        std::unique_ptr<Expr> Expression;

        ExprSttmt(std::unique_ptr<Expr> Expression)
            : Expression(std::move(Expression)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class IdentExpr : public Expr {
    public:
        std::string Name;

        IdentExpr(std::string Name) : Name(std::move(Name)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class ParExpr : public Expr {
    public:
        std::unique_ptr<Expr> Content;

        ParExpr(std::unique_ptr<Expr> Content) : Content(std::move(Content)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class ArrayAccessExpr : public Expr {
    public:
        std::string Name;
        std::unique_ptr<Expr> Index;

        ArrayAccessExpr(std::string Name, std::unique_ptr<Expr> Index)
            : Name(std::move(Name)), Index(std::move(Index)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class ArrayLiteralExpr : public Expr {
    public:
        std::vector<std::unique_ptr<Expr>> Elements;

        void addElement(std::unique_ptr<Expr> element) {
            Elements.push_back(std::move(element));
        }

        void accept(Visitor& v) override { v.visit(*this); }
    };

    /// Array repeat initializer: `[fill] * N`.
    /// Declares a fully-initialized array where every element is a copy of fill.
    class ArrayRepeatExpr : public Expr {
    public:
        std::unique_ptr<Expr> Fill;
        std::size_t           Count;

        ArrayRepeatExpr(std::unique_ptr<Expr> fill, std::size_t count)
            : Fill(std::move(fill)), Count(count) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class AssignExpr : public Expr {
    public:
        std::string AssignOp;
        std::unique_ptr<Expr> Assignee;
        std::unique_ptr<Expr> Assigned;

        AssignExpr(std::string AssignOp, std::unique_ptr<Expr> Assignee, std::unique_ptr<Expr> Assigned)
            : AssignOp(std::move(AssignOp)), Assignee(std::move(Assignee)), Assigned(std::move(Assigned)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class ForSttmt : public Sttmt {
    public:
        std::unique_ptr<Expr> Start;
        std::unique_ptr<Expr> Cond;
        std::unique_ptr<Expr> After;
        std::unique_ptr<BlockSttmt> Then;

        ForSttmt(std::unique_ptr<Expr> Start, std::unique_ptr<Expr> Cond, std::unique_ptr<Expr> After, std::unique_ptr<BlockSttmt> Then)
            : Start(std::move(Start)), Cond(std::move(Cond)), After(std::move(After)), Then(std::move(Then)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class ImportSttmt : public Sttmt {
    public:
        std::string Import;

        ImportSttmt(std::string Import) : Import(std::move(Import)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class FStringExpr : public Expr {
    public:
        struct Segment {
            std::string text;
            std::unique_ptr<Expr> expr;
        };
        std::vector<Segment> Parts;

        void addText(const std::string& t) { Parts.push_back({t, nullptr}); }
        void addExpr(std::unique_ptr<Expr> e) { Parts.push_back({"", std::move(e)}); }
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class UnaryExpr : public Expr {
    public:
        std::string Op;
        std::unique_ptr<Expr> Operand;

        UnaryExpr(std::string op, std::unique_ptr<Expr> operand)
            : Op(std::move(op)), Operand(std::move(operand)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class SaiSttmt : public Sttmt {
    public:
        std::unique_ptr<Expr> Code;

        SaiSttmt(std::unique_ptr<Expr> code) : Code(std::move(code)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    class KonfirmaSttmt : public Sttmt {
    public:
        std::unique_ptr<Expr> Cond;

        KonfirmaSttmt(std::unique_ptr<Expr> cond) : Cond(std::move(cond)) {}
        void accept(Visitor& v) override { v.visit(*this); }
    };

    /// Strips any number of ParExpr wrappers and returns the underlying
    /// IdentExpr, or nullptr if the expression is not a bare identifier.
    inline IdentExpr* unwrapIdentExpr(Expr* expr) {
        while (auto* par = dynamic_cast<ParExpr*>(expr))
            expr = par->Content.get();
        return dynamic_cast<IdentExpr*>(expr);
    }

}
}

#endif // _KRIOL_AST_TRANSPILER_HEADER
