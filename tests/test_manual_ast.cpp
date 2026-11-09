#include <iostream>
#include <cassert>
#include <memory>
#include "../src/ast/AstVisitor.hpp"
#include "../src/ast/StmtNodes.hpp"
#include "../src/ast/ExprNodes.hpp"

// Visitor de teste para validar o percurso da AST
class TestCountingVisitor : public AstVisitor {
public:
    int total_nos_visitados{0};

    void visit(ProgramNode& node) override {
        total_nos_visitados++;
        if (node.bloco_principal) node.bloco_principal->accept(*this);
    }

    void visit(BlockStmtNode& node) override {
        total_nos_visitados++;
        for (auto& cmd : node.comandos) {
            cmd->accept(*this);
        }
    }

    void visit(VarDeclStmtNode& node) override {
        total_nos_visitados++;
        if (node.inicializador) node.inicializador->accept(*this);
    }

    void visit(AssignStmtNode& node) override {
        total_nos_visitados++;
        if (node.expressao) node.expressao->accept(*this);
    }

    void visit(OutputStmtNode& node) override {
        total_nos_visitados++;
        for (auto& argumento : node.argumentos) {
            if (argumento) argumento->accept(*this);
        }
    }

    void visit(InputStmtNode&) override { total_nos_visitados++; }
    void visit(IfStmtNode& node) override {
        total_nos_visitados++;
        if (node.condicao) node.condicao->accept(*this);
        if (node.entao_bloco) node.entao_bloco->accept(*this);
        if (node.senao_bloco) node.senao_bloco->accept(*this);
    }
    void visit(WhileStmtNode& node) override {
        total_nos_visitados++;
        if (node.condicao) node.condicao->accept(*this);
        if (node.corpo) node.corpo->accept(*this);
    }
    void visit(ForStmtNode& node) override {
        total_nos_visitados++;
        if (node.inicializacao) node.inicializacao->accept(*this);
        if (node.condicao) node.condicao->accept(*this);
        if (node.incremento) node.incremento->accept(*this);
        if (node.corpo) node.corpo->accept(*this);
    }
    void visit(BreakStmtNode&) override { total_nos_visitados++; }
    void visit(ContinueStmtNode&) override { total_nos_visitados++; }
    void visit(LiteralExprNode&) override { total_nos_visitados++; }
    void visit(IdentifierExprNode&) override { total_nos_visitados++; }
    void visit(UnaryExprNode& node) override {
        total_nos_visitados++;
        if (node.operando) node.operando->accept(*this);
    }
    void visit(BinaryExprNode& node) override {
        total_nos_visitados++;
        if (node.esquerda) node.esquerda->accept(*this);
        if (node.direita) node.direita->accept(*this);
    }
};

int main() {
    // Montando manualmente:
    // {
    //     inteiro x = 10 + 5;
    //     mostre(x);
    // }
    auto bloco = std::make_unique<BlockStmtNode>();

    // 10 + 5
    auto lit10 = std::make_unique<LiteralExprNode>(10, TypeKind::Inteiro);
    auto lit5 = std::make_unique<LiteralExprNode>(5, TypeKind::Inteiro);
    auto soma = std::make_unique<BinaryExprNode>(BinaryOp::Soma, std::move(lit10), std::move(lit5));

    // inteiro x = (10 + 5)
    auto decl = std::make_unique<VarDeclStmtNode>(TypeKind::Inteiro, "x", std::move(soma));
    bloco->add(std::move(decl));

    // mostre(x)
    auto varX = std::make_unique<IdentifierExprNode>("x");
    auto saida = std::make_unique<OutputStmtNode>(std::move(varX));
    bloco->add(std::move(saida));

    // Raiz do programa
    auto programa = std::make_unique<ProgramNode>(std::move(bloco));

    // Percorre a árvore com o visitor
    TestCountingVisitor visitor;
    programa->accept(visitor);

    // Validações com assert
    // Total esperado de nós: ProgramNode(1) + BlockStmtNode(1) + VarDecl(1) + BinaryExpr(1) + Lit10(1) + Lit5(1) + OutputStmt(1) + IdentX(1) = 8 nós
    assert(visitor.total_nos_visitados == 8);

    std::cout << "[SUCESSO] AST montada manualmente e percorrida com sucesso!" << std::endl;
    std::cout << "Total de nos visitados pelo AstVisitor: " << visitor.total_nos_visitados << std::endl;

    return 0;
}
