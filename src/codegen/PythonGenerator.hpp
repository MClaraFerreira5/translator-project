#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "../ast/AstVisitor.hpp"
#include "../ast/ExprNodes.hpp"
#include "../ast/StmtNodes.hpp"
#include "../common/TypeKind.hpp"

namespace oxente {

// Indica uma AST que viola o contrato de entrada do gerador (por exemplo, nome
// não declarado). Programas Oxente inválidos devem ser rejeitados antes, pela
// análise semântica; esta exceção denuncia erro interno do tradutor.
class ErroGeracao : public std::logic_error {
public:
    using std::logic_error::logic_error;
};

// Traduz uma AST validada para Python 3.
//
// Contratos do código gerado:
// - Cada declaração recebe o nome interno v<N>_<nome>, em que N conta as
//   declarações daquele nome. O formato é injetivo, não colide com palavras
//   reservadas, builtins ou helpers _ox_* e isola blocos independentes, já que
//   if/while não criam escopo em Python.
// - se_num_for é representado por um IfStmtNode como único comando do
//   senao_bloco do ramo anterior e vira elif.
// - A condição de num_para e arrodeia é de parada: while not (condicao).
// - simbora em arrodeia executa a atualização antes do continue.
class PythonGenerator : public AstVisitor {
public:
    // Retorna o módulo Python completo. Lança ErroGeracao se a AST for inválida.
    std::string gerar(ProgramNode& programa);

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
    struct Expressao {
        std::string codigo;
        int precedencia;
        TypeKind tipo;
    };

    struct Variavel {
        std::string nome_python;
        TypeKind tipo;
    };

    using Linhas = std::vector<std::string>;

    Expressao avaliar(ExprNode* expressao, const char* descricao);
    std::string avaliarConvertido(ExprNode* expressao, TypeKind destino, const char* descricao);
    Expressao chamadaRuntime(const char* helper, const Expressao& esquerda, const Expressao& direita,
                             int linha, TypeKind tipo) const;

    const Variavel& declarar(const std::string& nome, TypeKind tipo);
    const Variavel& buscar(const std::string& nome) const;

    void linha(const std::string& texto);
    void emitirCorpo(BlockStmtNode* corpo, const Linhas& apos_corpo = {});
    Linhas capturar(StmtNode& comando);

    std::string saida_;
    int nivel_{0};
    Expressao resultado_{};
    std::vector<std::unordered_map<std::string, Variavel>> escopos_;
    std::map<std::string, int> ocorrencias_;
    // Um item por laço aberto, do externo ao interno. Aponta para as linhas da
    // atualização de arrodeia; é nullptr em num_para.
    std::vector<const Linhas*> lacos_;
};

} // namespace oxente
