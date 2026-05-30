#include "../../include/kriol/sema.hh"

#include <stdexcept>

using namespace kriol::ast;

namespace kriol {
namespace sema {

void SemanticAnalyzer::Check(BlockSttmt* program) {
    if (!program) return;
    pushScope();
    for (auto& s : program->SttmtList)
        if (s) s->accept(*this);
    popScope();
}


bool SemanticAnalyzer::blockDefinitelyReturns(BlockSttmt* block) const {
    if (!block) return false;
    for (auto& stmt : block->SttmtList) {
        if (!stmt) continue;

        // Direct return
        if (dynamic_cast<ReturnSttmt*>(stmt.get()))
            return true;

        // Nested braced block that itself always returns
        if (auto* nested = dynamic_cast<BlockSttmt*>(stmt.get()))
            if (blockDefinitelyReturns(nested)) return true;

        // if/else where both branches always return
        if (auto* ifStmt = dynamic_cast<IfSttmt*>(stmt.get())) {
            if (ifStmt->Else &&
                blockDefinitelyReturns(ifStmt->Then.get()) &&
                blockDefinitelyReturns(ifStmt->Else.get()))
                return true;
        }
    }
    return false;
}


void SemanticAnalyzer::visit(VarDeclSttmt& node) {
    if (!node.IsParam) {
        // Check for duplicate in the innermost scope only
        if (!SymbolScopes.empty()) {
            auto& cur = SymbolScopes.back();
            if (cur.count(node.Name))
                addError("variable '" + node.Name + "' already declared in this scope");
        }
        declareVar(node.Name, node.Type);
    }
    if (node.Value) node.Value->accept(*this);
}

void SemanticAnalyzer::visit(BlockSttmt& node) {
    // A braced compound_statement introduces a new scope; a bare block
    // (program root, function body) uses the scope pushed by the caller.
    if (node.BracketsOn) pushScope();
    for (auto& s : node.SttmtList)
        if (s) s->accept(*this);
    if (node.BracketsOn) popScope();
}

void SemanticAnalyzer::visit(FuncArgs& node) {
    // Handled inside FuncDeclSttmt
}

void SemanticAnalyzer::visit(FuncDeclSttmt& node) {
    // Register before visiting body so recursive calls are valid
    FunctionTable[node.Name] = node.Type;

    std::string savedRetType = CurrFuncRetType;
    std::string savedName    = CurrFuncName;
    CurrFuncRetType = node.Type;
    CurrFuncName    = node.Name;

    pushScope();

    // Add parameters to function scope
    if (node.Args) {
        for (auto& arg : node.Args->Args) {
            if (arg) declareVar(arg->Name, arg->Type);
        }
    }

    if (node.Body) node.Body->accept(*this);

    // Check that non-void, non-entry functions return on all paths
    bool isEntry = (node.Name == "inisiu");
    bool isVoid  = (node.Type == "vaziu");
    if (!isEntry && !isVoid && !blockDefinitelyReturns(node.Body.get()))
        addError("function '" + node.Name + "' does not return on all paths");

    popScope();
    CurrFuncRetType = savedRetType;
    CurrFuncName    = savedName;
}

void SemanticAnalyzer::visit(IfSttmt& node) {
    if (node.Cond) node.Cond->accept(*this);
    if (node.Then) node.Then->accept(*this);
    if (node.Else) node.Else->accept(*this);
}

void SemanticAnalyzer::visit(WhileSttmt& node) {
    if (node.Cond) node.Cond->accept(*this);
    ++LoopDepth;
    if (node.Do) node.Do->accept(*this);
    --LoopDepth;
}

void SemanticAnalyzer::visit(JumpSttmt& node) {
    // ReturnSttmt overrides this, so here we only see break/continue
    if (LoopDepth == 0)
        addError("'" + node.Name + "' used outside a loop");
}

void SemanticAnalyzer::visit(ReturnSttmt& node) {
    // Make sure we are not returning a value from a void function
    if (!CurrFuncRetType.empty() && CurrFuncRetType == "vaziu" && node.ReturnValue)
        addError("returning a value from void function '" + CurrFuncName + "'");
    // Make sure we are returning a value from a non-void function
    if (!CurrFuncRetType.empty() && CurrFuncRetType != "vaziu" && !node.ReturnValue)
        addError("missing return value in non-void function '" + CurrFuncName + "'");
    if (node.ReturnValue) node.ReturnValue->accept(*this);
}

void SemanticAnalyzer::visit(FuncCallArgs& node) {
    // Handled inside FunCallExpr
}

void SemanticAnalyzer::visit(FunCallExpr& node) {
    if (node.Args)
        for (auto& arg : node.Args->Args)
            if (arg) arg->accept(*this);
}

void SemanticAnalyzer::visit(BinExpr& node) {
    if (node.LHS) node.LHS->accept(*this);
    if (node.RHS) node.RHS->accept(*this);
}

void SemanticAnalyzer::visit(LiteralExpr& node) {
    // Nothing to check for literal constants
}

void SemanticAnalyzer::visit(ExprSttmt& node) {
    if (node.Expression) node.Expression->accept(*this);
}

void SemanticAnalyzer::visit(IdentExpr& node) {
    // Only flag as undefined if we are inside a function scope (SymbolScopes
    // has at least 2 levels: top-level scope + function scope).
    if (SymbolScopes.size() >= 2) {
        if (!lookupVar(node.Name))
            addError("undefined variable name '" + node.Name + "'");
    }
}

void SemanticAnalyzer::visit(ParExpr& node) {
    if (node.Content) node.Content->accept(*this);
}

void SemanticAnalyzer::visit(AssignExpr& node) {
    if (node.Assignee) node.Assignee->accept(*this);
    if (node.Assigned) node.Assigned->accept(*this);
}

void SemanticAnalyzer::visit(ForSttmt& node) {
    if (node.Start) node.Start->accept(*this);
    if (node.Cond)  node.Cond->accept(*this);
    ++LoopDepth;
    if (node.Then)  node.Then->accept(*this);
    --LoopDepth;
    if (node.After) node.After->accept(*this);
}

void SemanticAnalyzer::visit(MostraFunCallExpr& node) {
    if (node.Args)
        for (auto& arg : node.Args->Args)
            if (arg) arg->accept(*this);
}

void SemanticAnalyzer::visit(ImportSttmt& node) {
    throw std::runtime_error("'inpristan' (import) statements are not supported yet");
}

} // namespace sema
} // namespace kriol
