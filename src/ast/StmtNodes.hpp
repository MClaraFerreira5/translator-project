#pragma once
#include <memory>
#include <vector>
#include <string>
#include "AstNode.hpp"
#include "AstVisitor.hpp"
#include "ExprNodes.hpp"
#include "../common/TypeKind.hpp"

// Classe base para comandos
class StmtNode : public AstNode {
    using AstNode::AstNode;
};

// Bloco de comandos: { ... }
class BlockStmtNode : public StmtNode {
public:
    std::vector<std::unique_ptr<StmtNode>> comandos;
    using StmtNode::StmtNode;

    void add(std::unique_ptr<StmtNode> cmd) {
        comandos.push_back(std::move(cmd));
    }

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Programa principal (nó raiz da AST)
class ProgramNode : public AstNode {
public:
    std::unique_ptr<BlockStmtNode> bloco_principal;

    explicit ProgramNode(std::unique_ptr<BlockStmtNode> bloco, SourceLocation l = {0, 0})
        : AstNode(l), bloco_principal(std::move(bloco)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Declaração de variável: inteiro x = 10
class VarDeclStmtNode : public StmtNode {
public:
    TypeKind tipo;
    std::string nome;
    std::unique_ptr<ExprNode> inicializador; // Pode ser nullptr se não inicializada

    VarDeclStmtNode(TypeKind t, std::string n, std::unique_ptr<ExprNode> init = nullptr, SourceLocation l = {0, 0})
        : StmtNode(l), tipo(t), nome(std::move(n)), inicializador(std::move(init)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Atribuição: x = 20
class AssignStmtNode : public StmtNode {
public:
    std::string nome;
    std::unique_ptr<ExprNode> expressao;

    AssignStmtNode(std::string n, std::unique_ptr<ExprNode> expr, SourceLocation l = {0, 0})
        : StmtNode(l), nome(std::move(n)), expressao(std::move(expr)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Saída: mostre / escreva
class OutputStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> expressao;

    explicit OutputStmtNode(std::unique_ptr<ExprNode> expr, SourceLocation l = {0, 0})
        : StmtNode(l), expressao(std::move(expr)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Entrada: leia
class InputStmtNode : public StmtNode {
public:
    std::string nome_variavel;

    explicit InputStmtNode(std::string var, SourceLocation l = {0, 0})
        : StmtNode(l), nome_variavel(std::move(var)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Condicional: se (cond) bloco_entao senao bloco_senao
class IfStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condicao;
    std::unique_ptr<BlockStmtNode> entao_bloco;
    std::unique_ptr<BlockStmtNode> senao_bloco; // Pode ser nullptr

    IfStmtNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockStmtNode> entao, std::unique_ptr<BlockStmtNode> senao = nullptr, SourceLocation l = {0, 0})
        : StmtNode(l), condicao(std::move(cond)), entao_bloco(std::move(entao)), senao_bloco(std::move(senao)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Laço: enquanto
class WhileStmtNode : public StmtNode {
public:
    std::unique_ptr<ExprNode> condicao;
    std::unique_ptr<BlockStmtNode> corpo;

    WhileStmtNode(std::unique_ptr<ExprNode> cond, std::unique_ptr<BlockStmtNode> corpo, SourceLocation l = {0, 0})
        : StmtNode(l), condicao(std::move(cond)), corpo(std::move(corpo)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Laço: arrodeia (for)
class ForStmtNode : public StmtNode {
public:
    std::unique_ptr<StmtNode> inicializacao;
    std::unique_ptr<ExprNode> condicao;
    std::unique_ptr<StmtNode> incremento;
    std::unique_ptr<BlockStmtNode> corpo;

    ForStmtNode(std::unique_ptr<StmtNode> init, std::unique_ptr<ExprNode> cond, std::unique_ptr<StmtNode> inc, std::unique_ptr<BlockStmtNode> corpo, SourceLocation l = {0, 0})
        : StmtNode(l), inicializacao(std::move(init)), condicao(std::move(cond)), incremento(std::move(inc)), corpo(std::move(corpo)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// canso (break)
class BreakStmtNode : public StmtNode {
public:
    using StmtNode::StmtNode;
    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// simbora (continue)
class ContinueStmtNode : public StmtNode {
public:
    using StmtNode::StmtNode;
    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};