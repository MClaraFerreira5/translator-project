#include "semantic/SemanticAnalyzer.hpp"
#include "ast/StmtNodes.hpp"
#include "ast/ExprNodes.hpp"
#include <iostream>
#include <cassert>

using namespace oxente;

int main() {
    std::cout << "[TESTE SEMANTICA] Iniciando verificacoes semanticas..." << std::endl;

    // 1. Promoção válida: inteiro -> tronxo
    {
        DiagnosticEngine engine("", false);
        SemanticAnalyzer sem(&engine);

        // inteiro a bota 10
        auto lit10 = std::make_unique<LiteralExprNode>(10, TypeKind::Inteiro);
        auto declA = std::make_unique<VarDeclStmtNode>(TypeKind::Inteiro, "a", std::move(lit10));
        declA->accept(sem);

        // tronxo b bota a (válido por promoção)
        auto varA = std::make_unique<IdentifierExprNode>("a");
        auto declB = std::make_unique<VarDeclStmtNode>(TypeKind::Tronxo, "b", std::move(varA));
        declB->accept(sem);

        assert(!sem.tem_erros);
    }

    // 2. Rejeição de conversão implícita inversa: tronxo -> inteiro
    {
        DiagnosticEngine engine("", false);
        SemanticAnalyzer sem(&engine);

        // inteiro x bota 7,5 (deve falhar!)
        auto litTronxo = std::make_unique<LiteralExprNode>(7.5, TypeKind::Tronxo);
        auto declX = std::make_unique<VarDeclStmtNode>(TypeKind::Inteiro, "x", std::move(litTronxo));
        declX->accept(sem);

        assert(sem.tem_erros);
    }

    // 3. Operadores Aritméticos e Anotação de tipo_calculado
    {
        DiagnosticEngine engine("", false);
        SemanticAnalyzer sem(&engine);

        // 2 + 3 -> Inteiro
        auto soma = std::make_unique<BinaryExprNode>(BinaryOp::Soma,
                    std::make_unique<LiteralExprNode>(2, TypeKind::Inteiro),
                    std::make_unique<LiteralExprNode>(3, TypeKind::Inteiro));
        soma->accept(sem);
        assert(soma->tipo_calculado == TypeKind::Inteiro);

        // 2 + 3,5 -> Tronxo
        auto somaMista = std::make_unique<BinaryExprNode>(BinaryOp::Soma,
                         std::make_unique<LiteralExprNode>(2, TypeKind::Inteiro),
                         std::make_unique<LiteralExprNode>(3.5, TypeKind::Tronxo));
        somaMista->accept(sem);
        assert(somaMista->tipo_calculado == TypeKind::Tronxo);

        // 10 / 2 -> sempre Tronxo
        auto divReal = std::make_unique<BinaryExprNode>(BinaryOp::Divisao,
                       std::make_unique<LiteralExprNode>(10, TypeKind::Inteiro),
                       std::make_unique<LiteralExprNode>(2, TypeKind::Inteiro));
        divReal->accept(sem);
        assert(divReal->tipo_calculado == TypeKind::Tronxo);

        // abs(-5) -> preserva Inteiro
        auto unNeg = std::make_unique<UnaryExprNode>(UnaryOp::Negativo,
                     std::make_unique<LiteralExprNode>(5, TypeKind::Inteiro));
        auto opAbs = std::make_unique<UnaryExprNode>(UnaryOp::Abs, std::move(unNeg));
        opAbs->accept(sem);
        assert(opAbs->tipo_calculado == TypeKind::Inteiro);
    }

    // 4. Divisão Inteira (//) e Resto (%) rejeitam tronxo
    {
        DiagnosticEngine engine("", false);
        SemanticAnalyzer sem(&engine);

        auto divIntInvalida = std::make_unique<BinaryExprNode>(BinaryOp::DivisaoInteira,
                              std::make_unique<LiteralExprNode>(10.5, TypeKind::Tronxo),
                              std::make_unique<LiteralExprNode>(2, TypeKind::Inteiro));
        divIntInvalida->accept(sem);
        assert(sem.tem_erros);
    }

    // 5. Condição não booleana em vai_que (deve falhar)
    {
        DiagnosticEngine engine("", false);
        SemanticAnalyzer sem(&engine);

        // vai_que (10) bora { }
        auto ifStmt = std::make_unique<IfStmtNode>(
            std::make_unique<LiteralExprNode>(10, TypeKind::Inteiro),
            std::make_unique<BlockStmtNode>()
        );
        ifStmt->accept(sem);
        assert(sem.tem_erros);
    }

    // 6. canso e simbora fora de laço (deve falhar)
    {
        DiagnosticEngine engine("", false);
        SemanticAnalyzer sem(&engine);

        auto brk = std::make_unique<BreakStmtNode>();
        brk->accept(sem);
        assert(sem.tem_erros);

        auto cont = std::make_unique<ContinueStmtNode>();
        cont->accept(sem);
        assert(sem.tem_erros);
    }

    // 7. canso dentro de laço (válido)
    {
        DiagnosticEngine engine("", false);
        SemanticAnalyzer sem(&engine);

        auto corpo = std::make_unique<BlockStmtNode>();
        corpo->add(std::make_unique<BreakStmtNode>());

        auto loop = std::make_unique<WhileStmtNode>(
            std::make_unique<LiteralExprNode>(true, TypeKind::Papo),
            std::move(corpo)
        );
        loop->accept(sem);
        assert(!sem.tem_erros);
    }

    std::cout << "[SUCESSO] Todos os testes semanticos passaram perfeitamente!" << std::endl;
    return 0;
}