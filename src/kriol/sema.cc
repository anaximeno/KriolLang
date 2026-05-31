#include "../../include/kriol/sema.hh"

#include <stdexcept>
#include <unordered_set>

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
                addError("variable '" + node.Name + "' already declared in this scope"
                         + (node.LineNum ? " (line " + std::to_string(node.LineNum) + ")" : ""));
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
        addError("function '" + node.Name + "' does not return on all paths"
                 + (node.LineNum ? " (line " + std::to_string(node.LineNum) + ")" : ""));

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
        addError("'" + node.Name + "' used outside a loop"
                 + (node.LineNum ? " (line " + std::to_string(node.LineNum) + ")" : ""));
}

void SemanticAnalyzer::visit(ReturnSttmt& node) {
    // Make sure we are not returning a value from a void function
    if (!CurrFuncRetType.empty() && CurrFuncRetType == "vaziu" && node.ReturnValue)
        addError("returning a value from void function '" + CurrFuncName + "'"
                 + (node.LineNum ? " (line " + std::to_string(node.LineNum) + ")" : ""));
    // Make sure we are returning a value from a non-void function
    if (!CurrFuncRetType.empty() && CurrFuncRetType != "vaziu" && !node.ReturnValue)
        addError("missing return value in non-void function '" + CurrFuncName + "'"
                 + (node.LineNum ? " (line " + std::to_string(node.LineNum) + ")" : ""));
    if (node.ReturnValue) node.ReturnValue->accept(*this);
}

void SemanticAnalyzer::visit(FuncCallArgs& node) {
    // Handled inside FunCallExpr
}

void SemanticAnalyzer::visit(FunCallExpr& node) {
    if (node.Args)
        for (auto& arg : node.Args->Args)
            if (arg) arg->accept(*this);
    // Resolve return type from FunctionTable
    auto it = FunctionTable.find(node.Name);
    if (it != FunctionTable.end())
        node.ResolvedType = it->second;
}

void SemanticAnalyzer::visit(BinExpr& node) {
    if (node.LHS) node.LHS->accept(*this);
    if (node.RHS) node.RHS->accept(*this);

    // Comparison and logical operators always yield bool
    static const std::unordered_set<std::string> boolOps = {
        "==", "!=", "<", "<=", ">", ">=", "&&", "||"
    };

    if (boolOps.count(node.Op)) {
        node.ResolvedType = "bool";
    } else {
        // Arithmetic: promote nter+num -> num, otherwise keep operand type
        std::string lt = node.LHS ? node.LHS->ResolvedType : "";
        std::string rt = node.RHS ? node.RHS->ResolvedType : "";
        if (lt == "num" || rt == "num")
            node.ResolvedType = "num";
        else if (!lt.empty())
            node.ResolvedType = lt;
        else
            node.ResolvedType = rt;
    }
}

void SemanticAnalyzer::visit(LiteralExpr& node) {
    if (node.Type == "char*") node.ResolvedType = "textu";
    else                      node.ResolvedType = node.Type;
}

void SemanticAnalyzer::visit(ExprSttmt& node) {
    if (node.Expression) {
        node.Expression->accept(*this);
        node.ResolvedType = node.Expression->ResolvedType;
    }
}

void SemanticAnalyzer::visit(IdentExpr& node) {
    // Only flag as undefined if we are inside a function scope (SymbolScopes
    // has at least 2 levels: top-level scope + function scope).
    if (SymbolScopes.size() >= 2) {
        if (!lookupVar(node.Name))
            addError("undefined variable name '" + node.Name + "'"
                     + (node.LineNum ? " (line " + std::to_string(node.LineNum) + ")" : ""));
    }
    auto t = lookupVar(node.Name);
    if (t) node.ResolvedType = *t;
}

void SemanticAnalyzer::visit(ParExpr& node) {
    if (node.Content) {
        node.Content->accept(*this);
        node.ResolvedType = node.Content->ResolvedType;
    }
}

void SemanticAnalyzer::visit(AssignExpr& node) {
    if (node.Assignee) node.Assignee->accept(*this);
    if (node.Assigned) {
        node.Assigned->accept(*this);
        node.ResolvedType = node.Assigned->ResolvedType;
    }
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
    node.ResolvedType = "vaziu";
}

void SemanticAnalyzer::visit(ImportSttmt& node) {
    throw std::runtime_error("'inpristan' (import) statements are not supported yet");
}

void SemanticAnalyzer::visit(FStringExpr& node) {
    static const std::unordered_set<std::string> printableTypes = {
        "nter", "num", "bool", "textu"
    };
    for (auto& seg : node.Parts) {
        if (seg.expr) {
            seg.expr->accept(*this);
            const std::string& t = seg.expr->ResolvedType;
            if (!t.empty() && !printableTypes.count(t))
                addError("f-string interpolation: cannot format value of type '" + t + "'"
                         + (node.LineNum ? " (line " + std::to_string(node.LineNum) + ")" : ""));
        }
    }
    node.ResolvedType = "textu";
}

void SemanticAnalyzer::visit(UnaryExpr& node) {
    if (node.Operand) node.Operand->accept(*this);
    if (node.Op == "!")
        node.ResolvedType = "bool";
    else // "-" (numeric negation) keeps operand type, default to num if unknown
        node.ResolvedType = node.Operand ? node.Operand->ResolvedType : "num";
}

void SemanticAnalyzer::visit(SaiSttmt& node) {
    if (node.Code) {
        node.Code->accept(*this);
        const std::string& t = node.Code->ResolvedType;
        if (t != "nter") {
            std::string err = "sai() expects an integer exit code";
            if (!t.empty()) err += ", got value of type '" + t + "'";
            addError(err + (node.LineNum ? " (line " + std::to_string(node.LineNum) + ")" : ""));
        }
    }
}

void SemanticAnalyzer::visit(KonfirmaSttmt& node) {
    if (node.Cond) {
        node.Cond->accept(*this);
        const std::string& t = node.Cond->ResolvedType;
        if (t != "bool" && t != "nter" && t != "num") {
            std::string err = "konfirma() expects a boolean condition";
            if (!t.empty()) err += ", got value of type '" + t + "'";
            addError(err + (node.LineNum ? " (line " + std::to_string(node.LineNum) + ")" : ""));
        }
    };
}

} // namespace sema
} // namespace kriol
