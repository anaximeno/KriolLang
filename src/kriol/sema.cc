#include "../../include/kriol/sema.hh"
#include "../../include/kriol/type_utils.hh"

#include <stdexcept>
#include <unordered_set>
#include <algorithm>

using namespace kriol::ast;

namespace kriol {
namespace sema {

namespace {

using kriol::typeutils::arrayElementType;
using kriol::typeutils::firstArrayDim;
using kriol::typeutils::isArrayType;

}

bool SemanticAnalyzer::handleArrayIdentArg(ast::Expr& expr) {
    auto* ident = unwrapIdentExpr(&expr);
    if (!ident) return false;

    auto t = lookupVar(ident->Name);
    if (!t) {
        addError(errLoc(ident->LineNum) + "undefined variable name '" + ident->Name + "'");
        return true;
    }

    if (!isArrayType(*t)) return false;

    auto* init = lookupInitState(ident->Name);
    if (init && init->isArray && !init->fullyInitialized)
        addError(errLoc(ident->LineNum) + "array '" + ident->Name + "' may contain uninitialized elements");

    ident->ResolvedType = *t;
    expr.ResolvedType = *t;
    return true;
}

static const std::unordered_set<std::string> reservedKeywords = {
    // Built-in/runtime statements and call forms.
    "mostra", "mostran", "sai", "konfirma",

    // Language keywords.
    "si", "sinon", "nkuantu", "pa", "fn", "divolvi", "inpristan",
    "para", "kontinua", "dipoz",

    // Type names and literals.
    "num", "nter", "bool", "textu", "sin", "nau"
};

bool SemanticAnalyzer::isReservedKeyword(const std::string& name) {
    return reservedKeywords.count(name) != 0;
}

bool SemanticAnalyzer::checkDeclaredNameValid(const std::string& name,
                                            const std::string& kind,
                                            int lineNum) {
    if (!isReservedKeyword(name)) return true;

    addError(errLoc(lineNum) + kind + " name '" + name
             + "' is reserved and cannot be redeclared");

    return false;
}

bool SemanticAnalyzer::isWideningCoercion(const std::string& from,
                                          const std::string& to) {
    if (from == to) return true;
    if (from == "nter" && to == "num")  return true;
    if (from == "bool" && to == "nter") return true;
    if (from == "bool" && to == "num")  return true;
    return false;
}

void SemanticAnalyzer::registerFuncSignature(FuncDeclSttmt& node) {
    if (!checkDeclaredNameValid(node.Name, "function", node.LineNum)) return;

    if (FunctionTable.count(node.Name)) {
        addError(errLoc(node.LineNum) + "duplicate function declaration '" + node.Name + "'");
        return;
    }

    FuncInfo info;
    info.retType = node.Type;
    if (node.Args)
        for (auto& arg : node.Args->Args)
            if (arg) info.paramTypes.push_back(arg->Type);
    FunctionTable[node.Name] = std::move(info);
}

void SemanticAnalyzer::Check(BlockSttmt* program) {
    if (!program) return;

    // First pass: register all top-level function signatures so forward calls
    // and mutual recursion are resolved before any body is visited.
    for (auto& s : program->SttmtList) {
        if (auto* fn = dynamic_cast<FuncDeclSttmt*>(s.get()))
            registerFuncSignature(*fn);
    }

    // Second pass: full semantic walk.
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
    const std::string kind = node.IsParam ? "parameter" : "variable";
    bool canDeclare = checkDeclaredNameValid(node.Name, kind, node.LineNum);
    VarInitState initState;
    initState.isArray = node.IsArray;

    if (node.IsArray && node.ArraySize == 0) {
        addError(errLoc(node.LineNum) + "array variable '" + node.Name + "' must have a positive size");
        canDeclare = false;
    }

    if (node.IsArray && node.Value) {
        auto* initLit = dynamic_cast<ArrayLiteralExpr*>(node.Value.get());
        auto* initRep = dynamic_cast<ArrayRepeatExpr*>(node.Value.get());
        if (!initLit && !initRep) {
            addError(errLoc(node.LineNum) + "array variable '" + node.Name + "' must use an array initializer like [a, b, c] or [value] * N");
            canDeclare = false;
        } else if (initRep) {
            initRep->accept(*this);
            const std::size_t expected = node.ArraySize;
            if (initRep->Count != expected) {
                addError(errLoc(node.LineNum) + "array variable '" + node.Name + "' has size "
                         + std::to_string(expected) + " but repeat initializer [value] * "
                         + std::to_string(initRep->Count) + " has a different count");
                canDeclare = false;
            }
            const std::string elemType = arrayElementType(node.Type);
            const auto& fillType = initRep->Fill ? initRep->Fill->ResolvedType : "";
            if (!fillType.empty() && fillType != elemType && !isWideningCoercion(fillType, elemType))
                addError(errLoc(node.LineNum) + "repeat initializer fill type '" + fillType
                         + "' does not match array element type '" + elemType + "'");
            initState.elementInitialized.assign(expected, true);
            initState.fullyInitialized = true;
        } else {
            initLit->accept(*this);
            const std::size_t expected = node.ArraySize;
            const std::size_t got = initLit->Elements.size();
            initState.elementInitialized.assign(expected, false);
            if (got != expected) {
                addError(errLoc(node.LineNum) + "array variable '" + node.Name + "' expects "
                         + std::to_string(expected) + " initializer element(s), got " + std::to_string(got));
                canDeclare = false;
            }

            const std::string elemType = arrayElementType(node.Type);
            for (size_t i = 0; i < initLit->Elements.size(); ++i) {
                const auto& t = initLit->Elements[i]->ResolvedType;
                if (!t.empty() && t != elemType && !isWideningCoercion(t, elemType)) {
                    addError(errLoc(node.LineNum) + "array initializer element " + std::to_string(i + 1)
                             + ": expected '" + elemType + "', got '" + t + "'");
                }
                if (i < initState.elementInitialized.size()) initState.elementInitialized[i] = true;
            }
            initState.fullyInitialized = (got == expected);
        }
    } else if (node.IsArray) {
        initState.elementInitialized.assign(node.ArraySize, false);
        initState.fullyInitialized = false;
    } else {
        initState.fullyInitialized = node.IsParam || (node.Value != nullptr);
    }

    // Check for duplicate in the innermost scope only.
    if (!SymbolScopes.empty()) {
        auto& cur = SymbolScopes.back();
        if (cur.count(node.Name)) {
            addError(errLoc(node.LineNum) + kind + " '" + node.Name + "' already declared in this scope");
            canDeclare = false;
        }
    }

    if (canDeclare)
        declareVar(node.Name, node.Type);

    if (canDeclare)
        declareInitState(node.Name, initState);

    if (node.Value) {
        node.Value->accept(*this);
        if (!node.Value->ResolvedType.empty() && node.Value->ResolvedType == "vaziu")
            addError(errLoc(node.LineNum) + "cannot assign void expression to variable '" + node.Name + "'");
    }
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
    // If the signature wasn't pre-registered (nested function), register it now.
    // Top-level functions are already in FunctionTable from the first pass.
    if (!FunctionTable.count(node.Name)) {
        // Top-level functions were handled in the pre-registration pass.
        if (SymbolScopes.size() > 1)
            registerFuncSignature(node);
    }

    std::string savedRetType = CurrFuncRetType;
    std::string savedName    = CurrFuncName;
    CurrFuncRetType = node.Type;
    CurrFuncName    = node.Name;

    pushScope();

    // Add parameters to function scope
    if (node.Args) {
        for (auto& arg : node.Args->Args) {
            if (arg) arg->accept(*this);
        }
    }

    if (node.Body) node.Body->accept(*this);

    // Check that non-void, non-entry functions return on all paths
    bool isEntry = (node.Name == "inisiu");
    bool isVoid  = (node.Type == "vaziu");
    if (!isEntry && !isVoid && !blockDefinitelyReturns(node.Body.get()))
        addError(errLoc(node.LineNum) + "function '" + node.Name + "' does not return on all paths");

    popScope();
    CurrFuncRetType = savedRetType;
    CurrFuncName    = savedName;
}

void SemanticAnalyzer::visit(IfSttmt& node) {
    if (node.Cond) {
        node.Cond->accept(*this);
        if (node.Cond->ResolvedType == "vaziu")
            addError(errLoc(node.LineNum) + "void expression cannot be used as a condition");
    }
    if (node.Then) node.Then->accept(*this);
    if (node.Else) node.Else->accept(*this);
}

void SemanticAnalyzer::visit(WhileSttmt& node) {
    if (node.Cond) {
        node.Cond->accept(*this);
        if (node.Cond->ResolvedType == "vaziu")
            addError(errLoc(node.LineNum) + "void expression cannot be used as a condition");
    }
    ++LoopDepth;
    if (node.Do) node.Do->accept(*this);
    --LoopDepth;
}

void SemanticAnalyzer::visit(JumpSttmt& node) {
    // ReturnSttmt overrides this, so here we only see break/continue
    if (LoopDepth == 0)
        addError(errLoc(node.LineNum) + "'" + node.Name + "' used outside a loop");
}

void SemanticAnalyzer::visit(ReturnSttmt& node) {
    const std::string loc = errLoc(node.LineNum);

    if (!CurrFuncRetType.empty() && CurrFuncRetType == "vaziu" && node.ReturnValue)
        addError(loc + "returning a value from void function '" + CurrFuncName + "'");

    if (!CurrFuncRetType.empty() && CurrFuncRetType != "vaziu" && !node.ReturnValue)
        addError(loc + "missing return value in non-void function '" + CurrFuncName + "'");

    if (node.ReturnValue) {
        node.ReturnValue->accept(*this);
        const std::string& got = node.ReturnValue->ResolvedType;
        if (!got.empty() && !CurrFuncRetType.empty()
                && CurrFuncRetType != "vaziu"
                && got != CurrFuncRetType
                && !isWideningCoercion(got, CurrFuncRetType))
            addError(loc + "returning '" + got + "' from function '" + CurrFuncName
                     + "' declared as '" + CurrFuncRetType + "'");
    }
}

void SemanticAnalyzer::visit(FuncCallArgs& node) {
    // Handled inside FunCallExpr
}

void SemanticAnalyzer::visit(FunCallExpr& node) {
    if (node.Args)
        for (auto& arg : node.Args->Args)
            if (arg) arg->accept(*this);

    auto it = FunctionTable.find(node.Name);
    if (it == FunctionTable.end()) {
        addError(errLoc(node.LineNum) + "undeclared function '" + node.Name + "'");
        return;
    }

    const FuncInfo& info = it->second;
    node.ResolvedType = info.retType;

    const std::string loc = errLoc(node.LineNum);
    size_t got  = node.Args ? node.Args->Args.size() : 0;
    size_t want = info.paramTypes.size();

    if (got != want) {
        addError(loc + "function '" + node.Name + "' expects " + std::to_string(want)
                 + " argument(s), got " + std::to_string(got));
        return; // type checks make no sense if counts differ
    }

    if (node.Args) {
        for (size_t i = 0; i < node.Args->Args.size(); ++i) {
            const std::string& argType   = node.Args->Args[i]->ResolvedType;
            const std::string& paramType = info.paramTypes[i];
            if (!argType.empty() && argType != paramType
                    && !isWideningCoercion(argType, paramType))
                addError(loc + "argument " + std::to_string(i + 1) + " of '" + node.Name
                         + "': expected '" + paramType + "', got '" + argType + "'");
        }
    }
}

void SemanticAnalyzer::visit(BinExpr& node) {
    if (node.LHS) node.LHS->accept(*this);
    if (node.RHS) node.RHS->accept(*this);

    const std::string lt = node.LHS ? node.LHS->ResolvedType : "";
    const std::string rt = node.RHS ? node.RHS->ResolvedType : "";

    if (lt == "vaziu")
        addError(errLoc(node.LineNum) + "void expression cannot be used as an operand");
    if (rt == "vaziu")
        addError(errLoc(node.LineNum) + "void expression cannot be used as an operand");

    // Comparison and logical operators always yield bool
    static const std::unordered_set<std::string> boolOps = {
        "==", "!=", "<", "<=", ">", ">=", "&&", "||"
    };

    if (boolOps.count(node.Op)) {
        node.ResolvedType = "bool";
    } else {
        // Arithmetic: promote nter+num -> num, otherwise keep operand type
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
            addError(errLoc(node.LineNum) + "undefined variable name '" + node.Name + "'");
    }
    auto t = lookupVar(node.Name);
    if (t) {
        node.ResolvedType = *t;
        if (isArrayType(*t))
            addError(errLoc(node.LineNum) + "array variable '" + node.Name + "' must be indexed");

        auto* init = lookupInitState(node.Name);
        if (init && !init->isArray && !init->fullyInitialized)
            addError(errLoc(node.LineNum) + "use of uninitialized variable '" + node.Name + "'");
    }
}

void SemanticAnalyzer::visit(ParExpr& node) {
    if (node.Content) {
        node.Content->accept(*this);
        node.ResolvedType = node.Content->ResolvedType;
    }
}

void SemanticAnalyzer::visit(ArrayAccessExpr& node) {
    if (node.Index) {
        node.Index->accept(*this);
        const std::string& indexType = node.Index->ResolvedType;
        if (!indexType.empty() && indexType != "nter")
            addError(errLoc(node.LineNum) + "array index for '" + node.Name + "' must be an integer");
    }

    auto arrayType = lookupVar(node.Name);
    if (!arrayType) {
        addError(errLoc(node.LineNum) + "undefined array name '" + node.Name + "'");
        return;
    }

    if (!isArrayType(*arrayType)) {
        addError(errLoc(node.LineNum) + "'" + node.Name + "' is not an array");
        return;
    }

    node.ResolvedType = arrayElementType(*arrayType);

    auto firstDim = firstArrayDim(*arrayType);
    if (!firstDim || *firstDim == 0) {
        addError(errLoc(node.LineNum) + "array '" + node.Name + "' has invalid size metadata");
    }

    auto* init = lookupInitState(node.Name);
    if (!init || !init->isArray) return;
    if (init->fullyInitialized) return;

    auto* lit = dynamic_cast<LiteralExpr*>(node.Index.get());
    if (!lit || lit->Type != "nter") {
        addError(errLoc(node.LineNum) + "array '" + node.Name + "' may contain uninitialized elements");
        return;
    }

    long long idx = 0;
    try {
        idx = std::stoll(lit->Value);
    } catch (...) {
        addError(errLoc(node.LineNum) + "array index for '" + node.Name + "' must be an integer literal");
        return;
    }

    if (idx < 0 || static_cast<std::size_t>(idx) >= init->elementInitialized.size()) {
        // Bounds diagnostics are handled elsewhere.
        return;
    }

    if (!init->elementInitialized[static_cast<std::size_t>(idx)])
        addError(errLoc(node.LineNum) + "array element '" + node.Name + "[" + std::to_string(idx) + "]' is not initialized");
}

void SemanticAnalyzer::visit(ArrayLiteralExpr& node) {
    for (auto& element : node.Elements) {
        if (element) element->accept(*this);
    }
    node.ResolvedType = "array_literal";
}

void SemanticAnalyzer::visit(ArrayRepeatExpr& node) {
    if (node.Fill) node.Fill->accept(*this);
    node.ResolvedType = "array_repeat";
}

void SemanticAnalyzer::visit(AssignExpr& node) {
    std::string assigneeType;
    bool canMarkInitialized = true;

    if (auto* ident = dynamic_cast<IdentExpr*>(node.Assignee.get())) {
        auto t = lookupVar(ident->Name);
        if (!t) {
            addError(errLoc(node.LineNum) + "undefined variable name '" + ident->Name + "'");
            canMarkInitialized = false;
        } else {
            assigneeType = *t;
            if (isArrayType(assigneeType)) {
                addError(errLoc(node.LineNum) + "cannot assign directly to array variable '" + ident->Name + "'; assign to an index");
                canMarkInitialized = false;
            }
        }
    } else if (auto* arr = dynamic_cast<ArrayAccessExpr*>(node.Assignee.get())) {
        if (arr->Index) {
            arr->Index->accept(*this);
            if (arr->Index->ResolvedType != "nter") {
                addError(errLoc(node.LineNum) + "array index for '" + arr->Name + "' must be an integer");
                canMarkInitialized = false;
            }
        }

        auto t = lookupVar(arr->Name);
        if (!t) {
            addError(errLoc(node.LineNum) + "undefined array name '" + arr->Name + "'");
            canMarkInitialized = false;
        } else if (!isArrayType(*t)) {
            addError(errLoc(node.LineNum) + "'" + arr->Name + "' is not an array");
            canMarkInitialized = false;
        } else {
            assigneeType = arrayElementType(*t);
        }
    } else if (node.Assignee) {
        node.Assignee->accept(*this);
        assigneeType = node.Assignee->ResolvedType;
    }

    if (node.Assigned) {
        node.Assigned->accept(*this);
        node.ResolvedType = node.Assigned->ResolvedType;

        if (!assigneeType.empty() && !node.Assigned->ResolvedType.empty()
                && assigneeType != node.Assigned->ResolvedType
                && !isWideningCoercion(node.Assigned->ResolvedType, assigneeType)) {
            addError(errLoc(node.LineNum) + "cannot assign value of type '" + node.Assigned->ResolvedType
                     + "' to target of type '" + assigneeType + "'");
            canMarkInitialized = false;
        }
    }

    if (!canMarkInitialized) return;

    if (auto* ident = dynamic_cast<IdentExpr*>(node.Assignee.get())) {
        auto* init = lookupInitStateMutable(ident->Name);
        if (init && !init->isArray) init->fullyInitialized = true;
    } else if (auto* arr = dynamic_cast<ArrayAccessExpr*>(node.Assignee.get())) {
        auto* init = lookupInitStateMutable(arr->Name);
        if (init && init->isArray && !init->fullyInitialized) {
            auto* lit = dynamic_cast<LiteralExpr*>(arr->Index.get());
            if (lit && lit->Type == "nter") {
                try {
                    long long idx = std::stoll(lit->Value);
                    if (idx >= 0 && static_cast<std::size_t>(idx) < init->elementInitialized.size()) {
                        init->elementInitialized[static_cast<std::size_t>(idx)] = true;
                        init->fullyInitialized = std::all_of(init->elementInitialized.begin(),
                                                             init->elementInitialized.end(),
                                                             [](bool v) { return v; });
                    }
                } catch (...) {
                    // NOTE: Non-integer literal is validated elsewhere.
                }
            }
        }
    }
}

void SemanticAnalyzer::visit(ForSttmt& node) {
    if (node.Start) node.Start->accept(*this);
    if (node.Cond) {
        node.Cond->accept(*this);
        if (node.Cond->ResolvedType == "vaziu")
            addError(errLoc(node.LineNum) + "void expression cannot be used as a condition");
    }
    ++LoopDepth;
    if (node.Then)  node.Then->accept(*this);
    --LoopDepth;
    if (node.After) node.After->accept(*this);
}

void SemanticAnalyzer::visit(MostraFunCallExpr& node) {
    if (node.Args)
        for (auto& arg : node.Args->Args)
            if (arg && !handleArrayIdentArg(*arg))
                arg->accept(*this);
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
        if (!seg.expr) continue;
        if (!handleArrayIdentArg(*seg.expr)) {
            seg.expr->accept(*this);
            const std::string& t = seg.expr->ResolvedType;
            if (!t.empty() && !printableTypes.count(t))
                addError(errLoc(node.LineNum) + "f-string interpolation: cannot format value of type '" + t + "'");
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
            addError(errLoc(node.LineNum) + err);
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
            addError(errLoc(node.LineNum) + err);
        }
    };
}

} // namespace sema
} // namespace kriol
