#include "../src/frontend/scanner.hpp"
#include "../src/ast/StmtNodes.hpp"
#include "../src/ast/ExprNodes.hpp"
#include "../src/common/DiagnosticEngine.hpp"
#include <iostream>
#include <memory>
#include <cassert>
#include <string>

namespace {

struct FecharArquivo { void operator()(FILE* arq) const { if (arq) std::fclose(arq); } };
using Arquivo = std::unique_ptr<FILE, FecharArquivo>;

Arquivo criar_fonte(const std::string& codigo) {
    Arquivo arq(std::tmpfile());
    if (!arq) throw std::runtime_error("Falha ao criar tmpfile");
    std::fwrite(codigo.data(), 1, codigo.size(), arq.get());
    std::rewind(arq.get());
    return arq;
}

std::unique_ptr<ProgramNode> parse_codigo(const std::string& codigo) {
    auto arq = criar_fonte(codigo);
    oxente::Scanner scanner(arq.get());
    oxente::DiagnosticEngine engine("teste.oxe", true);
    return oxente::parse(scanner, &engine);
}

bool parse_deve_falhar(const std::string& codigo) {
    try {
        auto arq = criar_fonte(codigo);
        oxente::Scanner scanner(arq.get());
        oxente::DiagnosticEngine engine("invalido.oxe", true);
        auto ast = oxente::parse(scanner, &engine);
        return ast == nullptr;
    } catch (const oxente::DiagnosticException&) {
        return true;
    } catch (const std::exception&) {
        return true;
    }
}

} // namespace

int main() {
    std::cout << "[TESTE PARSER] Iniciando verificações sintáticas..." << std::endl;

    // 1. Declaração, Atribuição e Localização
    {
        auto ast = parse_codigo("inteiro idade bota 25\nidade bota 30\n");
        assert(ast != nullptr);
        assert(ast->bloco_principal->comandos.size() == 2);

        auto decl = dynamic_cast<VarDeclStmtNode*>(ast->bloco_principal->comandos[0].get());
        assert(decl != nullptr);
        assert(decl->tipo == TypeKind::Inteiro);
        assert(decl->nome == "idade");
        assert(decl->loc.line == 1 && decl->loc.column == 1);

        auto lit = dynamic_cast<LiteralExprNode*>(decl->inicializador.get());
        assert(lit != nullptr && std::get<int>(lit->valor) == 25);

        auto atrib = dynamic_cast<AssignStmtNode*>(ast->bloco_principal->comandos[1].get());
        assert(atrib != nullptr && atrib->nome == "idade");
        assert(atrib->loc.line == 2 && atrib->loc.column == 1);
    }

    // 2. Precedência: Multiplicação antes de Soma
    {
        // 2 + 3 * 4 -> raiz deve ser Soma, filho direito Multiplicacao
        auto ast = parse_codigo("inteiro r bota 2 + 3 * 4\n");
        assert(ast != nullptr);
        auto decl = dynamic_cast<VarDeclStmtNode*>(ast->bloco_principal->comandos[0].get());
        assert(decl != nullptr);

        auto soma = dynamic_cast<BinaryExprNode*>(decl->inicializador.get());
        assert(soma != nullptr && soma->op == BinaryOp::Soma);

        auto lit2 = dynamic_cast<LiteralExprNode*>(soma->esquerda.get());
        assert(lit2 != nullptr && std::get<int>(lit2->valor) == 2);

        auto mult = dynamic_cast<BinaryExprNode*>(soma->direita.get());
        assert(mult != nullptr && mult->op == BinaryOp::Multiplicacao);
    }

    // 3. Precedência com Parênteses: (2 + 3) * 4
    {
        auto ast = parse_codigo("inteiro r bota (2 + 3) * 4\n");
        assert(ast != nullptr);
        auto decl = dynamic_cast<VarDeclStmtNode*>(ast->bloco_principal->comandos[0].get());
        auto mult = dynamic_cast<BinaryExprNode*>(decl->inicializador.get());
        assert(mult != nullptr && mult->op == BinaryOp::Multiplicacao);

        auto soma = dynamic_cast<BinaryExprNode*>(mult->esquerda.get());
        assert(soma != nullptr && soma->op == BinaryOp::Soma);
    }

    // 4. Associatividade de ** à direita: 2 ** 3 ** 2 -> 2 ** (3 ** 2)
    {
        auto ast = parse_codigo("tronxo pot bota 2 ** 3 ** 2\n");
        assert(ast != nullptr);
        auto decl = dynamic_cast<VarDeclStmtNode*>(ast->bloco_principal->comandos[0].get());
        auto pot1 = dynamic_cast<BinaryExprNode*>(decl->inicializador.get());
        assert(pot1 != nullptr && pot1->op == BinaryOp::Potencia);

        auto base = dynamic_cast<LiteralExprNode*>(pot1->esquerda.get());
        assert(base != nullptr && std::get<int>(base->valor) == 2);

        // O filho direito DEVE ser outra potência (associatividade à direita)
        auto pot2 = dynamic_cast<BinaryExprNode*>(pot1->direita.get());
        assert(pot2 != nullptr && pot2->op == BinaryOp::Potencia);
    }

    // 5. Unário e Potência: -2 ** 2 -> -(2 ** 2)
    {
        auto ast = parse_codigo("tronxo pot bota -2 ** 2\n");
        assert(ast != nullptr);
        auto decl = dynamic_cast<VarDeclStmtNode*>(ast->bloco_principal->comandos[0].get());
        auto neg = dynamic_cast<UnaryExprNode*>(decl->inicializador.get());
        assert(neg != nullptr && neg->op == UnaryOp::Negativo);

        auto pot = dynamic_cast<BinaryExprNode*>(neg->operando.get());
        assert(pot != nullptr && pot->op == BinaryOp::Potencia);
    }

    // 6. Expressão abs(...)
    {
        auto ast = parse_codigo("inteiro mod bota abs(-10)\n");
        assert(ast != nullptr);
        auto decl = dynamic_cast<VarDeclStmtNode*>(ast->bloco_principal->comandos[0].get());
        auto op_abs = dynamic_cast<UnaryExprNode*>(decl->inicializador.get());
        assert(op_abs != nullptr && op_abs->op == UnaryOp::Abs);

        auto neg = dynamic_cast<UnaryExprNode*>(op_abs->operando.get());
        assert(neg != nullptr && neg->op == UnaryOp::Negativo);
    }

    // 7. Entrada diga_ai e Saída desembucha com múltiplos argumentos (;)
    {
        auto ast = parse_codigo("diga_ai(x)\ndesembucha(vei Resultado tendeu; x + 1; evera)\n");
        assert(ast != nullptr);
        assert(ast->bloco_principal->comandos.size() == 2);

        auto entrada = dynamic_cast<InputStmtNode*>(ast->bloco_principal->comandos[0].get());
        assert(entrada != nullptr && entrada->nome_variavel == "x");

        auto saida = dynamic_cast<OutputStmtNode*>(ast->bloco_principal->comandos[1].get());
        assert(saida != nullptr);
        assert(saida->argumentos.size() == 3);

        auto arg1 = dynamic_cast<LiteralExprNode*>(saida->argumentos[0].get());
        assert(arg1 != nullptr && std::get<std::string>(arg1->valor) == "Resultado");
    }

    // 8. Rejeição de Casos Inválidos
    {
        // Comparação encadeada PROIBIDA: 1 < 2 < 3
        assert(parse_deve_falhar("papo r bota 1 < 2 < 3\n"));

        // Falta de identificador na declaração
        assert(parse_deve_falhar("inteiro bota 5\n"));

        // Falta de operando após '+'
        assert(parse_deve_falhar("inteiro r bota 10 +\n"));

        // Argumentos de desembucha separados por vírgula em vez de ';'
        assert(parse_deve_falhar("desembucha(1, 2)\n"));

        // Parêntese desbalanceado
        assert(parse_deve_falhar("inteiro r bota (2 + 3\n"));
    }

    std::cout << "[SUCESSO] Todos os testes do parser passaram perfeitamente!" << std::endl;
    return 0;
}