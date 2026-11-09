#pragma once

#include "ast/AstVisitor.hpp"
#include "ast/StmtNodes.hpp"
#include "ast/ExprNodes.hpp"
#include "SymbolTable.hpp"
#include "common/DiagnosticEngine.hpp"
#include <string>

namespace oxente {

class SemanticAnalyzer : public AstVisitor {
public:
    SymbolTable tabela;
    DiagnosticEngine* engine{nullptr};
    int profundidade_laco{0}; // Controla contexto para 'canso' e 'simbora'
    bool tem_erros{false};

    explicit SemanticAnalyzer(DiagnosticEngine* eng = nullptr)
        : tabela(eng), engine(eng) {}

    void error(SourceLocation loc, const std::string& causa, const std::string& msg) {
        tem_erros = true;
        if (engine) {
            engine->emitir(Stage::Semantico, loc, causa, msg);
        }
    }

    void type_error(SourceLocation loc, TypeKind esperado, TypeKind obtido) {
        tem_erros = true;
        if (engine) {
            engine->erro_misturou_as_coisas(loc, to_string(esperado), to_string(obtido));
        }
    }

    void erro_operador(BinaryExprNode& node, const char* exigencia) {
        const char* operador = "";
        switch (node.op) {
            case BinaryOp::Soma: operador = "+"; break;
            case BinaryOp::Subtracao: operador = "-"; break;
            case BinaryOp::Multiplicacao: operador = "*"; break;
            case BinaryOp::Divisao: operador = "/"; break;
            case BinaryOp::DivisaoInteira: operador = "//"; break;
            case BinaryOp::Modulo: operador = "%"; break;
            case BinaryOp::Potencia: operador = "**"; break;
            case BinaryOp::Menor: operador = "<"; break;
            case BinaryOp::MenorIgual: operador = "<="; break;
            case BinaryOp::Maior: operador = ">"; break;
            case BinaryOp::MaiorIgual: operador = ">="; break;
            case BinaryOp::Igual: operador = "=="; break;
            case BinaryOp::Diferente: operador = "!="; break;
            case BinaryOp::E: operador = "e"; break;
            case BinaryOp::Ou: operador = "ou"; break;
        }
        node.tipo_calculado = TypeKind::Erro;
        error(node.loc, "misturou_as_coisas",
              "Operador '" + std::string(operador) + "' exige " + exigencia +
              "; recebeu '" + to_string(node.esquerda->tipo_calculado) +
              "' e '" + to_string(node.direita->tipo_calculado) + "'.");
    }

    // --- COMANDOS ---

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
        if (!node.inicializador) {
            error(node.loc, "inicializador_ausente",
                  "Declaracao de '" + node.nome + "' exige inicializador.");
            return;
        }
        // 1. Analisa inicializador antes de registrar o nome (impede x bota x + 1)
        node.inicializador->accept(*this);
        TypeKind tipo_init = node.inicializador->tipo_calculado;

        // Valida compatibilidade: rejeita conversão implícita inversa (ex: tronxo para inteiro)
        if (tipo_init != TypeKind::Erro && !compatible_types(node.tipo, tipo_init)) {
            type_error(node.loc, node.tipo, tipo_init);
        }

        // 2. Insere na tabela de símbolos
        if (!tabela.inserir(node.nome, node.tipo, node.loc, true)) {
            tem_erros = true;
        }
    }

    void visit(AssignStmtNode& node) override {
        const Symbol* s = tabela.validar_uso(node.nome, node.loc);
        if (!s) {
            tem_erros = true;
        }

        if (node.expressao) {
            node.expressao->accept(*this);
            if (s && node.expressao->tipo_calculado != TypeKind::Erro &&
                !compatible_types(s->tipo, node.expressao->tipo_calculado)) {
                type_error(node.loc, s->tipo, node.expressao->tipo_calculado);
            }
        }
    }

    void visit(OutputStmtNode& node) override {
        for (auto& argumento : node.argumentos) {
            if (argumento) argumento->accept(*this);
        }
    }

    void visit(InputStmtNode& node) override {
        if (!tabela.validar_uso(node.nome_variavel, node.loc)) {
            tem_erros = true;
        }
    }

    void visit(IfStmtNode& node) override {
        if (node.condicao) {
            node.condicao->accept(*this);
            if (node.condicao->tipo_calculado != TypeKind::Papo && node.condicao->tipo_calculado != TypeKind::Erro) {
                type_error(node.condicao->loc, TypeKind::Papo, node.condicao->tipo_calculado);
            }
        }
        if (node.entao_bloco) node.entao_bloco->accept(*this);
        if (node.senao_bloco) node.senao_bloco->accept(*this);
    }

    void visit(WhileStmtNode& node) override {
        if (node.condicao) {
            node.condicao->accept(*this);
            if (node.condicao->tipo_calculado != TypeKind::Papo && node.condicao->tipo_calculado != TypeKind::Erro) {
                type_error(node.condicao->loc, TypeKind::Papo, node.condicao->tipo_calculado);
            }
        }
        profundidade_laco++;
        if (node.corpo) node.corpo->accept(*this);
        profundidade_laco--;
    }

    void visit(ForStmtNode& node) override {
        tabela.entrar_escopo(); // Escopo do arrodeia
        profundidade_laco++;

        if (node.inicializacao) node.inicializacao->accept(*this);
        if (node.condicao) {
            node.condicao->accept(*this);
            if (node.condicao->tipo_calculado != TypeKind::Papo && node.condicao->tipo_calculado != TypeKind::Erro) {
                type_error(node.condicao->loc, TypeKind::Papo, node.condicao->tipo_calculado);
            }
        }
        if (node.corpo) node.corpo->accept(*this);
        if (node.incremento) node.incremento->accept(*this);

        profundidade_laco--;
        tabela.sair_escopo();
    }

    void visit(BreakStmtNode& node) override {
        if (profundidade_laco <= 0) {
            error(node.loc, "contexto_invalido", "'canso' so pode ser usado dentro de lacos de repeticao.");
        }
    }

    void visit(ContinueStmtNode& node) override {
        if (profundidade_laco <= 0) {
            error(node.loc, "contexto_invalido", "'simbora' so pode ser usado dentro de lacos de repeticao.");
        }
    }

    // --- EXPRESSÕES (Cálculo e Anotação de tipo_calculado) ---

    void visit(LiteralExprNode& node) override {
    }

    void visit(IdentifierExprNode& node) override {
        const Symbol* s = tabela.validar_uso(node.nome, node.loc);
        if (s) {
            node.tipo_calculado = s->tipo;
        } else {
            node.tipo_calculado = TypeKind::Erro;
            tem_erros = true;
        }
    }

    void visit(UnaryExprNode& node) override {
        if (!node.operando) return;
        node.operando->accept(*this);
        TypeKind t = node.operando->tipo_calculado;
        if (t == TypeKind::Erro) {
            node.tipo_calculado = TypeKind::Erro;
            return;
        }

        if (node.op == UnaryOp::Positivo || node.op == UnaryOp::Negativo) {
            if (!is_numeric(t)) {
                type_error(node.loc, TypeKind::Inteiro, t);
                node.tipo_calculado = TypeKind::Erro;
            } else {
                node.tipo_calculado = t; // Preserva o tipo numérico
            }
        } else if (node.op == UnaryOp::Abs) {
            if (!is_numeric(t)) {
                type_error(node.loc, TypeKind::Inteiro, t);
                node.tipo_calculado = TypeKind::Erro;
            } else {
                node.tipo_calculado = t; // Preserva o tipo numérico
            }
        } else if (node.op == UnaryOp::Nao) {
            if (t != TypeKind::Papo) {
                type_error(node.loc, TypeKind::Papo, t);
                node.tipo_calculado = TypeKind::Erro;
            } else {
                node.tipo_calculado = TypeKind::Papo;
            }
        }
    }

    void visit(BinaryExprNode& node) override {
        if (!node.esquerda || !node.direita) return;
        node.esquerda->accept(*this);
        node.direita->accept(*this);

        TypeKind esq = node.esquerda->tipo_calculado;
        TypeKind dir = node.direita->tipo_calculado;

        if (esq == TypeKind::Erro || dir == TypeKind::Erro) {
            node.tipo_calculado = TypeKind::Erro;
            return;
        }

        switch (node.op) {
            // Aritméticos (+, -, *)
            case BinaryOp::Soma:
            case BinaryOp::Subtracao:
            case BinaryOp::Multiplicacao:
                if (!is_numeric(esq) || !is_numeric(dir)) {
                    erro_operador(node, "operandos numericos");
                } else if (esq == TypeKind::Tronxo || dir == TypeKind::Tronxo) {
                    node.tipo_calculado = TypeKind::Tronxo; // Promoção
                } else {
                    node.tipo_calculado = TypeKind::Inteiro;
                }
                break;

            // Divisão real (/) sempre resulta em tronxo
            case BinaryOp::Divisao:
                if (!is_numeric(esq) || !is_numeric(dir)) {
                    erro_operador(node, "operandos numericos");
                } else {
                    node.tipo_calculado = TypeKind::Tronxo;
                }
                break;

            // Potência (**) resulta em tronxo
            case BinaryOp::Potencia:
                if (!is_numeric(esq) || !is_numeric(dir)) {
                    erro_operador(node, "operandos numericos");
                } else {
                    node.tipo_calculado = TypeKind::Tronxo;
                }
                break;

            // Divisão inteira (//) e resto (%) exigem inteiros estritos
            case BinaryOp::DivisaoInteira:
            case BinaryOp::Modulo:
                if (esq != TypeKind::Inteiro || dir != TypeKind::Inteiro) {
                    erro_operador(node, "operandos do tipo inteiro");
                } else {
                    node.tipo_calculado = TypeKind::Inteiro;
                }
                break;

            // Relacionais (<, <=, >, >=)
            case BinaryOp::Menor:
            case BinaryOp::MenorIgual:
            case BinaryOp::Maior:
            case BinaryOp::MaiorIgual:
                if (!is_numeric(esq) || !is_numeric(dir)) {
                    erro_operador(node, "operandos numericos");
                } else {
                    node.tipo_calculado = TypeKind::Papo;
                }
                break;

            // Igualdade (==, !=)
            case BinaryOp::Igual:
            case BinaryOp::Diferente:
                if (is_numeric(esq) && is_numeric(dir)) {
                    node.tipo_calculado = TypeKind::Papo;
                } else if (esq == dir) {
                    node.tipo_calculado = TypeKind::Papo;
                } else {
                    erro_operador(node, "operandos de tipos compativeis");
                }
                break;

            // Lógicos (e, ou)
            case BinaryOp::E:
            case BinaryOp::Ou:
                if (esq != TypeKind::Papo || dir != TypeKind::Papo) {
                    erro_operador(node, "operandos do tipo papo");
                } else {
                    node.tipo_calculado = TypeKind::Papo;
                }
                break;
        }
    }
};

} // namespace oxente
