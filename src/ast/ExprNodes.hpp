#pragma once
#include <memory>
#include <string>
#include <variant>
#include "AstNode.hpp"
#include "AstVisitor.hpp"
#include "../common/TypeKind.hpp"
#include "../common/Operators.hpp"

// Classe base para todas as expressões
class ExprNode : public AstNode {
public:
    TypeKind tipo_calculado{TypeKind::Indefinido}; 
    using AstNode::AstNode;
};

// Literais (inteiro, tronxo, papo, resenha)
class LiteralExprNode : public ExprNode {
public:
    // Pode guardar int, double, bool ou string
    std::variant<int, double, bool, std::string> valor;

    LiteralExprNode(std::variant<int, double, bool, std::string> val, TypeKind tipo, SourceLocation l = {0, 0})
        : ExprNode(l), valor(std::move(val)) {
        tipo_calculado = tipo;
    }

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Identificador (nome de variável)
class IdentifierExprNode : public ExprNode {
public:
    std::string nome;

    IdentifierExprNode(std::string n, SourceLocation l = {0, 0})
        : ExprNode(l), nome(std::move(n)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Expressão Unária (+x, -x, nao x)
class UnaryExprNode : public ExprNode {
public:
    UnaryOp op;
    std::unique_ptr<ExprNode> operando;

    UnaryExprNode(UnaryOp op, std::unique_ptr<ExprNode> opnd, SourceLocation l = {0, 0})
        : ExprNode(l), op(op), operando(std::move(opnd)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};

// Expressão Binária (x + y, a == b)
class BinaryExprNode : public ExprNode {
public:
    BinaryOp op;
    std::unique_ptr<ExprNode> esquerda;
    std::unique_ptr<ExprNode> direita;

    BinaryExprNode(BinaryOp op, std::unique_ptr<ExprNode> esq, std::unique_ptr<ExprNode> dir, SourceLocation l = {0, 0})
        : ExprNode(l), op(op), esquerda(std::move(esq)), direita(std::move(dir)) {}

    void accept(AstVisitor& visitor) override { visitor.visit(*this); }
};