#include "../src/frontend/scanner.hpp"
#include "../src/ast/StmtNodes.hpp"
#include "../src/ast/ExprNodes.hpp"
#include "../src/common/DiagnosticEngine.hpp"
#include <iostream>
#include <memory>
#include <cassert>
#include <string>
#include <sstream>
#include "semantic/SemanticAnalyzer.hpp"

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
    // Uma exceção inesperada deve falhar o teste, não contar como rejeição.
    return parse_codigo(codigo) == nullptr;
}

void exigir(bool condicao, const std::string& mensagem) {
    if (!condicao) throw std::runtime_error(mensagem);
}

struct CapturarErros {
    std::ostringstream saida;
    std::streambuf* anterior = std::cerr.rdbuf(saida.rdbuf());
    ~CapturarErros() { std::cerr.rdbuf(anterior); }
};

void conferir_erro(const std::string& codigo, oxente::Stage etapa,
                   const std::string& causa, int linha, int coluna) {
    for (bool parar : {false, true}) {
        auto arq = criar_fonte(codigo);
        oxente::Scanner scanner(arq.get());
        oxente::DiagnosticEngine engine("invalido.oxe", parar);
        CapturarErros captura;
        auto ast = oxente::parse(scanner, &engine);
        exigir(ast == nullptr, "Erro retornou AST parcial");
        exigir(engine.tem_erros(), "Erro não registrado no motor");
        const auto mensagem = captura.saida.str();
        exigir(mensagem.find("[" + std::string(oxente::to_string(etapa)) + "]") != std::string::npos,
               "Etapa incorreta: " + mensagem);
        exigir(mensagem.find("invalido.oxe:" + std::to_string(linha) + ":" +
                             std::to_string(coluna) + ":") != std::string::npos,
               "Localização incorreta: " + mensagem);
        exigir(mensagem.find("erro '" + causa + "'") != std::string::npos,
               "Causa incorreta: " + mensagem);
        exigir(mensagem.find('\n') == mensagem.size() - 1, "Mais de um diagnóstico emitido");
    }
    auto arq = criar_fonte(codigo);
    oxente::Scanner scanner(arq.get());
    exigir(oxente::parse(scanner) == nullptr, "Erro sem motor retornou AST");
}

void conferir_erro_semantico(const std::string& codigo, const std::string& causa,
                            int linha, int coluna, const std::vector<std::string>& trechos) {
    for (bool parar : {false, true}) {
        auto ast = parse_codigo(codigo);
        exigir(ast != nullptr, "Sonda semântica rejeitada no parser");
        oxente::DiagnosticEngine motor("integracao.oxe", parar);
        oxente::SemanticAnalyzer analisador(&motor);
        CapturarErros captura;
        bool lancou = false;
        try {
            ast->accept(analisador);
        } catch (const oxente::DiagnosticException& erro) {
            lancou = true;
            exigir(erro.get().etapa == oxente::Stage::Semantico &&
                   erro.get().causa_tecnica == causa, "Diagnóstico incorreto na exceção");
            exigir(erro.get().loc.line == linha && erro.get().loc.column == coluna,
                   "Localização incorreta na exceção");
        }
        exigir(lancou == parar && motor.tem_erros(), "Sinalização semântica incorreta");
        if (!parar) exigir(analisador.tem_erros, "Visitante não marcou a falha");
        const auto mensagem = captura.saida.str();
        exigir(mensagem.find("[semântico]") != std::string::npos, "Etapa incorreta");
        exigir(mensagem.find("integracao.oxe:" + std::to_string(linha) + ":" +
                             std::to_string(coluna) + ":") != std::string::npos,
               "Localização incorreta");
        exigir(mensagem.find("erro '" + causa + "'") != std::string::npos, "Causa incorreta");
        for (const auto& trecho : trechos) {
            exigir(mensagem.find(trecho) != std::string::npos, "Diagnóstico sem: " + trecho);
        }
        exigir(mensagem.find('\n') == mensagem.size() - 1, "Mais de um diagnóstico emitido");
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

    // 9. Comandos exigem quebra de linha; EOF só termina o último comando.
    {
        conferir_erro("diga_ai(x) desembucha(x)", oxente::Stage::Sintatico,
                      "sintaxe_invalida", 1, 12);
        conferir_erro("inteiro x bota 1 inteiro y bota 2", oxente::Stage::Sintatico,
                      "sintaxe_invalida", 1, 18);
        conferir_erro("inteiro x bota 1 +\n2", oxente::Stage::Sintatico,
                      "sintaxe_invalida", 1, 19);

        for (const std::string quebra : {"\n", "\r\n"}) {
            for (const std::string final : {std::string{}, quebra}) {
                for (const std::string comentario : {"", " primeiramente fim"}) {
                    for (const std::string primeiro : {"diga_ai(x)", "inteiro x bota 1"}) {
                        const auto segundo = primeiro == "diga_ai(x)"
                            ? "desembucha(x; 2,5)" : "inteiro y bota 2";
                        auto ast = parse_codigo(quebra + primeiro + comentario + quebra +
                                                quebra + segundo + final);
                        exigir(ast && ast->bloco_principal->comandos.size() == 2,
                               "Sequência com término válido rejeitada");
                    }
                }
            }
        }
    }

    // 10. Programa e bloco começam no primeiro comando, sem contar linhas vazias.
    {
        for (const std::string inicio : {"", "\n", "\r\n\r\n\t"}) {
            auto ast = parse_codigo(inicio + "inteiro x bota 1");
            const int linha = inicio.empty() ? 1 : (inicio == "\n" ? 2 : 3);
            const int coluna = inicio == "\r\n\r\n\t" ? 2 : 1;
            exigir(ast && ast->loc.line == linha && ast->loc.column == coluna,
                   "Localização incorreta do programa");
            exigir(ast->bloco_principal->loc.line == linha &&
                   ast->bloco_principal->loc.column == coluna,
                   "Localização incorreta do bloco principal");
        }
        conferir_erro("", oxente::Stage::Sintatico, "sintaxe_invalida", 1, 1);
        conferir_erro("\n\n", oxente::Stage::Sintatico, "sintaxe_invalida", 3, 1);
    }

    // 11. A conversão respeita int/double da AST e não deixa exceções escaparem.
    {
        for (const std::string literal : {std::string("2147483648"),
                                          std::string(400, '9') + ",0"}) {
            // Inclui comando anterior e argumentos já construídos na pilha.
            conferir_erro("inteiro x bota 1\ndesembucha(x; " + literal + ")",
                          oxente::Stage::Lexico, "literal_fora_da_representacao", 2, 15);
        }
        auto ast = parse_codigo("inteiro x bota 2147483647\ntronxo y bota 7,5000");
        auto* decl = dynamic_cast<VarDeclStmtNode*>(ast->bloco_principal->comandos[1].get());
        auto* real = dynamic_cast<LiteralExprNode*>(decl->inicializador.get());
        exigir(real && std::get<double>(real->valor) == 7.5, "Vírgula decimal alterada");
    }

    // 12. Erros com árvore/pilha parciais e token de antecipação descartado.
    {
        conferir_erro("inteiro x bota 1\ndesembucha(x; 2 + )", oxente::Stage::Sintatico,
                      "sintaxe_invalida", 2, 19);
        conferir_erro("inteiro x bota 1\ndesembucha(x; 2 3)", oxente::Stage::Sintatico,
                      "sintaxe_invalida", 2, 17);
        conferir_erro("inteiro x bota 1\ndesembucha(x; 2 + @)", oxente::Stage::Lexico,
                      "erro_lexico", 2, 19);
        conferir_erro("inteiro x bota 1\ndesembucha(vei texto", oxente::Stage::Lexico,
                      "erro_lexico", 2, 12);
    }

    // 13. Reproduções OX-09: AST do parser real seguida do analisador semântico.
    {
        conferir_erro_semantico("inteiro x bota 1\ndesembucha(x; ausente)",
                                "nome_nao_declarado", 2, 15, {"ausente"});
        conferir_erro_semantico("desembucha(1 + evera)", "misturou_as_coisas", 1, 12,
                                {"'+'", "numericos", "'inteiro'", "'papo'"});
        conferir_erro_semantico("desembucha(8,5 // 2)", "misturou_as_coisas", 1, 12,
                                {"'//'", "tipo inteiro", "'tronxo'", "'inteiro'"});
        conferir_erro_semantico("inteiro x bota x + 1", "nome_nao_declarado", 1, 16, {"x"});
        conferir_erro("inteiro x", oxente::Stage::Sintatico, "sintaxe_invalida", 1, 10);

        auto ast = parse_codigo("inteiro x bota 1\ntronxo y bota x\npapo p bota evera\n"
                                "resenha r bota vei texto tendeu\ndesembucha(x; y; p; r)");
        exigir(ast != nullptr, "Saída válida rejeitada pelo parser");
        oxente::DiagnosticEngine motor("valido.oxe");
        oxente::SemanticAnalyzer analisador(&motor);
        ast->accept(analisador);
        exigir(!analisador.tem_erros && !motor.tem_erros(), "Saída válida rejeitada na semântica");
        auto* saida = dynamic_cast<OutputStmtNode*>(ast->bloco_principal->comandos.back().get());
        exigir(saida && saida->argumentos.size() == 4, "Argumentos de saída incorretos");
        const std::vector<TypeKind> tipos{TypeKind::Inteiro, TypeKind::Tronxo, TypeKind::Papo, TypeKind::Resenha};
        for (std::size_t i = 0; i < tipos.size(); ++i) {
            exigir(saida->argumentos[i]->tipo_calculado == tipos[i], "Argumento sem tipo resolvido");
        }
    }

    std::cout << "[SUCESSO] Testes do parser concluídos." << std::endl;
    return 0;
}
