#pragma once

#include "ast/AstVisitor.hpp"
#include "ast/StmtNodes.hpp"
#include "ast/ExprNodes.hpp"
#include "SymbolTable.hpp"

namespace oxente {

class ScopeVisitor : public AstVisitor {
public:
    SymbolTable tabela;
    bool houve_erro{false};

    explicit ScopeVisitor(DiagnosticEngine* engine = nullptr) : tabela(engine) {}

    void visit(ProgramNode& node) override {
        if (node.bloco_principal) {
            node.bloco_principal->accept(*this);
        }
    }

    void visit(BlockStmtNode& node) override {
        tabela.entrar_escopo();
        for (auto& cmd : node.comandos) {
            if (cmd) cmd->accept(*this);
        }
        tabela.sair_escopo();
    }

    void visit(VarDeclStmtNode& node) override {
        // 1. O inicializador é avaliado ANTES da variável ficar visível
        // Ex: "inteiro x bota x + 1" é inválido se x ainda não existia
        if (node.inicializador) {
            node.inicializador->accept(*this);
        }
        // 2. Registra o símbolo na tabela de símbolos
        if (!tabela.inserir(node.nome, node.tipo, node.loc, node.inicializador != nullptr)) {
            houve_erro = true;
        }
    }

    void visit(AssignStmtNode& node) override {
        // Valida se a variável atribuída existe e é visível
        if (!tabela.validar_uso(node.nome, node.loc)) {
            houve_erro = true;
        }
        if (node.expressao) {
            node.expressao->accept(*this);
        }
    }

    void visit(IdentifierExprNode& node) override {
        if (!tabela.validar_uso(node.nome, node.loc)) {
            houve_erro = true;
        }
    }

    void visit(OutputStmtNode& node) override {
        if (node.expressao) {
            node.expressao->accept(*this);
        }
    }

    void visit(InputStmtNode& node) override {
        if (!tabela.validar_uso(node.nome_variavel, node.loc)) {
            houve_erro = true;
        }
    }

    void visit(IfStmtNode& node) override {
        if (node.condicao) node.condicao->accept(*this);
        if (node.entao_bloco) node.entao_bloco->accept(*this);
        if (node.senao_bloco) node.senao_bloco->accept(*this);
    }

    void visit(WhileStmtNode& node) override {
        if (node.condicao) node.condicao->accept(*this);
        if (node.corpo) node.corpo->accept(*this);
    }

    // A inicialização cria um escopo próprio que envolve o laço todo!
    void visit(ForStmtNode& node) override {
        tabela.entrar_escopo(); // Abre escopo do arrodeia
        if (node.inicializacao) node.inicializacao->accept(*this);
        if (node.condicao) node.condicao->accept(*this);
        if (node.corpo) node.corpo->accept(*this);
        if (node.incremento) node.incremento->accept(*this);
        tabela.sair_escopo();   // Fecha escopo do arrodeia
    }

    void visit(UnaryExprNode& node) override {
        if (node.operando) node.operando->accept(*this);
    }

    void visit(BinaryExprNode& node) override {
        if (node.esquerda) node.esquerda->accept(*this);
        if (node.direita) node.direita->accept(*this);
    }

    void visit(LiteralExprNode&) override {}
    void visit(BreakStmtNode&) override {}
    void visit(ContinueStmtNode&) override {}
};

} // namespace oxente