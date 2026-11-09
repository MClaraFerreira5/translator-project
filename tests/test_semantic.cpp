#include "semantic/SemanticAnalyzer.hpp"
#include "ast/StmtNodes.hpp"
#include "ast/ExprNodes.hpp"
#include <iostream>
#include <cassert>
#include <sstream>
#include <vector>

using namespace oxente;

namespace {
void exigir(bool condicao, const std::string& mensagem) {
    if (!condicao) throw std::runtime_error(mensagem);
}

struct CapturarErros {
    std::ostringstream saida;
    std::streambuf* anterior = std::cerr.rdbuf(saida.rdbuf());
    ~CapturarErros() { std::cerr.rdbuf(anterior); }
};

void conferir_erro(AstNode& no, const std::string& causa, SourceLocation loc,
                  const std::vector<std::string>& trechos, bool interromper = false) {
    DiagnosticEngine motor("semantica.oxe", interromper);
    SemanticAnalyzer analisador(&motor);
    CapturarErros captura;
    bool lancou = false;
    try {
        no.accept(analisador);
    } catch (const DiagnosticException& erro) {
        lancou = true;
        exigir(erro.get().etapa == Stage::Semantico, "Etapa incorreta na exceção");
        exigir(erro.get().causa_tecnica == causa, "Causa incorreta na exceção");
        exigir(erro.get().loc.line == loc.line && erro.get().loc.column == loc.column,
               "Posição incorreta na exceção");
    }
    exigir(lancou == interromper, "Modo de interrupção incorreto");
    exigir(motor.tem_erros(), "Motor não sinalizou falha");
    if (!interromper) exigir(analisador.tem_erros, "Visitante não sinalizou falha");
    const auto mensagem = captura.saida.str();
    exigir(mensagem.find("[semântico]") != std::string::npos, "Etapa incorreta");
    exigir(mensagem.find("semantica.oxe:" + std::to_string(loc.line) + ":" +
                         std::to_string(loc.column) + ":") != std::string::npos,
           "Localização incorreta");
    exigir(mensagem.find("erro '" + causa + "'") != std::string::npos, "Causa incorreta");
    for (const auto& trecho : trechos) {
        exigir(mensagem.find(trecho) != std::string::npos, "Diagnóstico sem: " + trecho);
    }
    exigir(mensagem.find("'erro'") == std::string::npos, "Sentinela exposto como tipo encontrado");
    exigir(mensagem.find('\n') == mensagem.size() - 1, "Diagnóstico derivado inesperado");
}

class VisitanteOrdenado : public SemanticAnalyzer {
public:
    std::vector<std::string> nomes;
    using SemanticAnalyzer::visit;
    void visit(IdentifierExprNode& no) override {
        nomes.push_back(no.nome);
        SemanticAnalyzer::visit(no);
    }
};
}

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

    // 4. Cada operador é exercitado com inteiros e com incompatibilidade em cada lado.
    for (auto op : {BinaryOp::DivisaoInteira, BinaryOp::Modulo}) {
        SemanticAnalyzer sem;
        BinaryExprNode valida(op, std::make_unique<LiteralExprNode>(10, TypeKind::Inteiro),
                                  std::make_unique<LiteralExprNode>(2, TypeKind::Inteiro));
        valida.accept(sem);
        exigir(!sem.tem_erros && valida.tipo_calculado == TypeKind::Inteiro,
               "Divisão inteira/resto de inteiros rejeitados");
        for (bool esquerda_real : {false, true}) {
            for (bool interromper : {false, true}) {
                std::unique_ptr<ExprNode> real = std::make_unique<LiteralExprNode>(10.5, TypeKind::Tronxo);
                std::unique_ptr<ExprNode> inteiro = std::make_unique<LiteralExprNode>(2, TypeKind::Inteiro);
                BinaryExprNode invalida(op, esquerda_real ? std::move(real) : std::move(inteiro),
                                           esquerda_real ? std::move(inteiro) : std::move(real), {1, 12});
                conferir_erro(invalida, "misturou_as_coisas", {1, 12},
                              {op == BinaryOp::Modulo ? "'%'" : "'//'", "inteiro", "tronxo"}, interromper);
                exigir(invalida.tipo_calculado == TypeKind::Erro, "Sentinela não preservado");
            }
        }
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
        exigir(sem.tem_erros && engine.tem_erros(), "canso fora de laço foi aceito");
    }
    {
        DiagnosticEngine engine("", false);
        SemanticAnalyzer sem(&engine);
        auto cont = std::make_unique<ContinueStmtNode>();
        cont->accept(sem);
        exigir(sem.tem_erros && engine.tem_erros(), "simbora fora de laço foi aceito");
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

    // 8. Saída: primeiro argumento válido, segundo com nome ausente.
    for (bool interromper : {false, true}) {
        std::vector<std::unique_ptr<ExprNode>> argumentos;
        argumentos.push_back(std::make_unique<LiteralExprNode>(1, TypeKind::Inteiro));
        argumentos.push_back(std::make_unique<IdentifierExprNode>("ausente", SourceLocation{2, 15}));
        OutputStmtNode saida(std::move(argumentos), {2, 1});
        conferir_erro(saida, "nome_nao_declarado", {2, 15}, {"ausente"}, interromper);
    }
    {
        VisitanteOrdenado sem;
        exigir(sem.tabela.inserir("x", TypeKind::Inteiro, {1, 1}), "Falha ao declarar x");
        exigir(sem.tabela.inserir("y", TypeKind::Tronxo, {2, 1}), "Falha ao declarar y");
        std::vector<std::unique_ptr<ExprNode>> argumentos;
        for (const std::string nome : {"x", "y", "x"}) {
            argumentos.push_back(std::make_unique<IdentifierExprNode>(nome));
        }
        OutputStmtNode saida(std::move(argumentos));
        saida.accept(sem);
        exigir(!sem.tem_erros && sem.nomes == std::vector<std::string>{"x", "y", "x"},
               "Ordem de visita dos argumentos incorreta");
        exigir(saida.argumentos[1]->tipo_calculado == TypeKind::Tronxo, "Segundo argumento sem tipo");
    }

    // 9. Inicializador ausente é erro na declaração, mesmo sem uso posterior.
    for (bool interromper : {false, true}) {
        VarDeclStmtNode declaracao(TypeKind::Inteiro, "x", nullptr, {3, 4});
        conferir_erro(declaracao, "inicializador_ausente", {3, 4}, {"x", "inicializador"}, interromper);
    }
    {
        SemanticAnalyzer sem;
        VarDeclStmtNode declaracao(TypeKind::Inteiro, "x", nullptr, {3, 4});
        declaracao.accept(sem);
        exigir(sem.tem_erros && !sem.tabela.buscar("x"), "Declaração inválida foi inserida");
    }

    // 10. A mensagem inclui a operação, a exigência e os tipos reais dos dois lados.
    for (auto op : {BinaryOp::Soma, BinaryOp::Subtracao, BinaryOp::Multiplicacao,
                    BinaryOp::Divisao, BinaryOp::Potencia, BinaryOp::Menor,
                    BinaryOp::MenorIgual, BinaryOp::Maior, BinaryOp::MaiorIgual}) {
        for (bool interromper : {false, true}) {
            BinaryExprNode expr(op, std::make_unique<LiteralExprNode>(1, TypeKind::Inteiro),
                                   std::make_unique<LiteralExprNode>(true, TypeKind::Papo), {1, 12});
            conferir_erro(expr, "misturou_as_coisas", {1, 12}, {"Operador", "numericos", "'inteiro'", "'papo'"}, interromper);
            exigir(expr.tipo_calculado == TypeKind::Erro, "Expressão inválida perdeu o sentinela");
        }
    }
    for (auto op : {BinaryOp::Igual, BinaryOp::Diferente, BinaryOp::E, BinaryOp::Ou}) {
        BinaryExprNode expr(op, std::make_unique<LiteralExprNode>(1, TypeKind::Inteiro),
                               std::make_unique<LiteralExprNode>(true, TypeKind::Papo), {1, 12});
        conferir_erro(expr, "misturou_as_coisas", {1, 12}, {"Operador", "'inteiro'", "'papo'"});
    }

    // 11. O nome não fica visível no próprio inicializador; o erro se propaga.
    for (bool interromper : {false, true}) {
        VarDeclStmtNode declaracao(TypeKind::Inteiro, "x",
            std::make_unique<BinaryExprNode>(BinaryOp::Soma,
                std::make_unique<IdentifierExprNode>("x", SourceLocation{1, 16}),
                std::make_unique<LiteralExprNode>(1, TypeKind::Inteiro)), {1, 1});
        conferir_erro(declaracao, "nome_nao_declarado", {1, 16}, {"x"}, interromper);
    }

    std::cout << "[SUCESSO] Testes semânticos concluídos." << std::endl;
    return 0;
}
