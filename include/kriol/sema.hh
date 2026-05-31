#ifndef _KRIOL_SEMA_HEADER
#define _KRIOL_SEMA_HEADER

#include "ast.hh"
#include "constants.hh"

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>

namespace kriol {
namespace sema {

    class SemanticAnalyzer : public ast::Visitor {
    private:
        // Scoped symbol table: each entry is one scope level (name -> Kriol type)
        std::vector<std::unordered_map<std::string, std::string>> SymbolScopes;

        // Signature record for a user-defined function
        struct FuncInfo {
            std::string retType;
            std::vector<std::string> paramTypes;
        };

        // Known user-defined functions (name -> signature)
        std::unordered_map<std::string, FuncInfo> FunctionTable;

        // Return type of the function currently being analysed ("" at top level)
        std::string CurrFuncRetType;
        std::string CurrFuncName;

        // How many loops deep we currently are (used for break/continue validation)
        int LoopDepth = 0;

        // Collected errors
        std::vector<std::string> Errors;

        // --- scope helpers ---
        void pushScope() { SymbolScopes.push_back({}); }
        void popScope()  { if (!SymbolScopes.empty()) SymbolScopes.pop_back(); }

        void declareVar(const std::string& name, const std::string& type) {
            if (!SymbolScopes.empty())
                SymbolScopes.back()[name] = type;
        }

        // Returns the Kriol type of the variable if found in any scope, or empty string.
        std::optional<std::string> lookupVar(const std::string& name) const {
            for (auto it = SymbolScopes.rbegin(); it != SymbolScopes.rend(); ++it) {
                auto found = it->find(name);
                if (found != it->end()) return found->second;
            }
            return std::nullopt;
        }

        // Returns true if every reachable code path in the block ends with a
        // return statement. Conservative: only if/else with both branches
        // returning is recognised as a definite return.
        bool blockDefinitelyReturns(ast::BlockSttmt* block) const;

        // Returns true if assigning/returning `from` where `to` is expected
        // is a legal implicit widening (nter->num, bool->nter, bool->num).
        static bool isWideningCoercion(const std::string& from, const std::string& to);

        // Pre-registers a function's full signature into FunctionTable without
        // visiting the body. Called in the first pass of Check().
        void registerFuncSignature(ast::FuncDeclSttmt& node);

        void addError(const std::string& msg) { Errors.push_back(msg); }

    public:
        SemanticAnalyzer() = default;

        // Entry point: walk the whole program AST and collect errors.
        void Check(ast::BlockSttmt* program);

        bool HasErrors() const { return !Errors.empty(); }
        const std::vector<std::string>& GetErrors() const { return Errors; }

        // --- visitor overrides ---
        void visit(ast::VarDeclSttmt&      node) override;
        void visit(ast::BlockSttmt&        node) override;
        void visit(ast::FuncArgs&          node) override;
        void visit(ast::FuncDeclSttmt&     node) override;
        void visit(ast::IfSttmt&           node) override;
        void visit(ast::WhileSttmt&        node) override;
        void visit(ast::JumpSttmt&         node) override;
        void visit(ast::ReturnSttmt&       node) override;
        void visit(ast::FuncCallArgs&      node) override;
        void visit(ast::FunCallExpr&       node) override;
        void visit(ast::BinExpr&           node) override;
        void visit(ast::LiteralExpr&       node) override;
        void visit(ast::ExprSttmt&         node) override;
        void visit(ast::IdentExpr&         node) override;
        void visit(ast::ParExpr&           node) override;
        void visit(ast::AssignExpr&        node) override;
        void visit(ast::ForSttmt&          node) override;
        void visit(ast::MostraFunCallExpr& node) override;
        void visit(ast::ImportSttmt&       node) override;
        void visit(ast::FStringExpr&       node) override;
        void visit(ast::UnaryExpr&         node) override;
        void visit(ast::SaiSttmt&          node) override;
        void visit(ast::KonfirmaSttmt&     node) override;
    };

} // namespace sema
} // namespace kriol

#endif // _KRIOL_SEMA_HEADER
