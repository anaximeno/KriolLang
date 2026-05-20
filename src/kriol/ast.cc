#include "../../include/kriol/ast.hh"
#include "../../include/kriol/codegen.hh"
#include "../../include/kriol/cbinds.hh"

using namespace kriol::ast;

std::string translate_type(const std::string& type) {
    if (type == "nteru") return "int";
    if (type == "real") return "float";
    if (type == "vaziu") return "void";
    if (type == "bool") return "unsigned short";
    return type;
}


void CodeGenVisitor::visit(VarDeclSttmt& node) {
    os << translate_type(node.Type) << " " << node.Name;
    if (node.Value) {
        os << C_EQ;
        node.Value->accept(*this);
    }
}

void CodeGenVisitor::visit(BlockSttmt& node) {
    if (node.BracketsOn) os << C_OPEN_CURLY;
    for (auto& sttmt : node.SttmtList) {
        if (sttmt) sttmt->accept(*this);
    }
    if (node.BracketsOn) os << C_CLOSE_CURLY;
}

void CodeGenVisitor::visit(FuncArgs& node) {
    for (size_t i = 0; i < node.Args.size(); ++i) {
        if (node.Args[i]) node.Args[i]->accept(*this);
        if (i < node.Args.size() - 1) os << C_COMMA;
    }
}

void CodeGenVisitor::visit(FuncDeclSttmt& node) {
    // localizing the main entry point
    std::string name = node.Name;
    if (name == "inisiu") {
        name = "main";
    }

    os << translate_type(node.Type) << " " << name << C_OPEN_PAR;
    if (node.Args) node.Args->accept(*this);
    os << C_CLOSE_PAR;
    if (node.Body) node.Body->accept(*this);
}

void CodeGenVisitor::visit(IfSttmt& node) {
    os << C_IF << C_OPEN_PAR;
    if (node.Cond) node.Cond->accept(*this);
    os << C_CLOSE_PAR;
    if (node.Then) node.Then->accept(*this);
    if (node.Else) {
        os << " " << C_ELSE << " ";
        node.Else->accept(*this);
    }
}

void CodeGenVisitor::visit(WhileSttmt& node) {
    os << C_WHILE << C_OPEN_PAR;
    if (node.Cond) node.Cond->accept(*this);
    os << C_CLOSE_PAR;
    if (node.Do) node.Do->accept(*this);
}

void CodeGenVisitor::visit(JumpSttmt& node) {
    os << node.Name << C_SEMICOLON;
}

void CodeGenVisitor::visit(ReturnSttmt& node) {
    os << C_RETURN << " ";
    if (node.ReturnValue) node.ReturnValue->accept(*this);
    os << C_SEMICOLON;
}

void CodeGenVisitor::visit(FuncCallArgs& node) {
    for (size_t i = 0; i < node.Args.size(); ++i) {
        if (node.Args[i]) node.Args[i]->accept(*this);
        if (i < node.Args.size() - 1) os << C_COMMA;
    }
}

void CodeGenVisitor::visit(FunCallExpr& node) {
    os << node.Name << C_OPEN_PAR;
    if (node.Args) node.Args->accept(*this);
    os << C_CLOSE_PAR;
}

void CodeGenVisitor::visit(MostraFunCallExpr& node) {
    os << node.Name << C_OPEN_PAR;
    if (node.Args) node.Args->accept(*this);
    os << C_CLOSE_PAR;
}

void CodeGenVisitor::visit(BinExpr& node) {
    if (node.LHS) node.LHS->accept(*this);
    os << " " << node.Op << " ";
    if (node.RHS) node.RHS->accept(*this);
}

void CodeGenVisitor::visit(LiteralExpr& node) {
    if (node.AutoCast) {
        os << C_OPEN_PAR << translate_type(node.Type) << C_CLOSE_PAR;
    }
    os << node.Value;
}

void CodeGenVisitor::visit(ExprSttmt& node) {
    if (node.Expression) node.Expression->accept(*this);
    os << C_SEMICOLON;
}

void CodeGenVisitor::visit(IdentExpr& node) {
    os << node.Name;
}

void CodeGenVisitor::visit(ParExpr& node) {
    os << C_OPEN_PAR;
    if (node.Content) node.Content->accept(*this);
    os << C_CLOSE_PAR;
}

void CodeGenVisitor::visit(AssignExpr& node) {
    if (node.Assignee) node.Assignee->accept(*this);
    os << C_EQ;
    if (node.Assigned) node.Assigned->accept(*this);
}

void CodeGenVisitor::visit(ForSttmt& node) {
    os << C_FOR << C_OPEN_PAR;
    if (node.Start) node.Start->accept(*this);
    os << C_SEMICOLON;
    if (node.Cond) node.Cond->accept(*this);
    os << C_SEMICOLON;
    if (node.After) node.After->accept(*this);
    os << C_CLOSE_PAR;
    if (node.Then) node.Then->accept(*this);
}

void CodeGenVisitor::visit(ImportSttmt& node) {
    os << C_IMPORT << " " << node.Import << "\n";
}
