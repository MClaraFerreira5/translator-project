#pragma once

#include <ostream>
#include <string>

#include "AstVisitor.hpp"
#include "ExprNodes.hpp"
#include "StmtNodes.hpp"

namespace oxente {

// Escreve a AST como texto indentado, um nó por linha, para depuração (--dump-ast).
class AstPrinter : public AstVisitor {
public:
    explicit AstPrinter(std::ostream& saida) : saida_(saida) {}

    void visit(ProgramNode& node) override;
    void visit(BlockStmtNode& node) override;
    void visit(VarDeclStmtNode& node) override;
    void visit(AssignStmtNode& node) override;
    void visit(OutputStmtNode& node) override;
    void visit(InputStmtNode& node) override;
    void visit(IfStmtNode& node) override;
    void visit(WhileStmtNode& node) override;
    void visit(ForStmtNode& node) override;
    void visit(BreakStmtNode& node) override;
    void visit(ContinueStmtNode& node) override;
    void visit(LiteralExprNode& node) override;
    void visit(IdentifierExprNode& node) override;
    void visit(UnaryExprNode& node) override;
    void visit(BinaryExprNode& node) override;

private:
    void linha(const std::string& texto);
    void filho(AstNode* no);
    void secao(const char* rotulo, AstNode* no);

    std::ostream& saida_;
    int nivel_{0};
};

} // namespace oxente
