#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include "ast/AstPrinter.hpp"
#include "codegen/PythonGenerator.hpp"
#include "common/Diagnostic.hpp"
#include "common/DiagnosticEngine.hpp"
#include "frontend/scanner.hpp"
#include "oxente_runtime_embutido.hpp"
#include "semantic/SemanticAnalyzer.hpp"

namespace {
struct FecharArquivo {
    void operator()(FILE* arquivo) const { std::fclose(arquivo); }
};
int erro(const std::string& mensagem, int codigo = 2) {
    std::cerr << "oxente: " << mensagem << '\n';
    return codigo;
}
void ajuda() {
    std::cout << "Uso: oxente MODO arquivo.oxe [-o caminho]\n"
                 "  --help                 Exibe esta ajuda (usado sozinho).\n"
                 "  --check                Valida o programa, sem gerar código.\n"
                 "  --dump-ast             Exibe a AST como texto indentado.\n"
                 "  --emit-python          Gera Python 3 em stdout, ou no caminho de -o.\n"
                 "  -o, --output caminho   Saída exclusiva de --emit-python.\n"
                 "Exija um único modo e um arquivo .oxe; não há modo padrão.\n"
                 "Use -- antes de caminhos iniciados por hífen.\n"
                 "Programas inválidos não geram arquivos.\n"
                 "Códigos: 0 sucesso; 1 erro no programa; 2 argumentos; 3 entrada ou saída.\n";
}
}
int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "--help") {
        ajuda();
        return 0;
    }
    std::string modo, entrada, caminho_saida;
    bool fim_opcoes = false, tem_saida = false, tem_entrada = false;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (!fim_opcoes && arg == "--") { fim_opcoes = true; continue; }
        if (!fim_opcoes && (arg == "--check" || arg == "--dump-ast" || arg == "--emit-python")) {
            if (!modo.empty()) return erro("informe apenas um modo, sem repetições.");
            modo = arg;
        } else if (!fim_opcoes && (arg == "-o" || arg == "--output")) {
            if (tem_saida) return erro("opção de saída repetida.");
            tem_saida = true;
            if (i + 1 == argc || std::string(argv[i + 1]).empty() || argv[i + 1][0] == '-')
                return erro("falta caminho após " + arg + "; para nomes com hífen, use ./nome.");
            caminho_saida = argv[++i];
        } else if (!fim_opcoes && !arg.empty() && arg[0] == '-') {
            return erro("opção desconhecida ou incompatível: " + arg + ". Consulte --help.");
        } else {
            if (tem_entrada) return erro("informe apenas um arquivo de entrada.");
            tem_entrada = true;
            entrada = arg;
        }
    }
    if (modo.empty()) return erro("informe um modo explícito. Consulte --help.");
    if (entrada.empty()) return erro("falta arquivo de entrada .oxe.");
    if (tem_saida && modo != "--emit-python") return erro("-o/--output exige --emit-python.");
    const std::filesystem::path caminho(entrada);
    if (caminho.extension() != ".oxe") return erro("a entrada deve ter extensão .oxe.");
    std::error_code ec;
    if (!std::filesystem::is_regular_file(caminho, ec))
        return erro("entrada inexistente ou não é arquivo regular: " + entrada, 3);
    if (tem_saida && std::filesystem::equivalent(caminho, caminho_saida, ec))
        return erro("a saída não pode ser o próprio arquivo de entrada.", 2);
    std::unique_ptr<FILE, FecharArquivo> fonte(std::fopen(entrada.c_str(), "rb"));
    if (!fonte) return erro("não foi possível abrir a entrada: " + entrada, 3);
    // Prepara o motor de diagnóstico para o arquivo atual
    oxente::DiagnosticEngine engine(entrada, true); // true = para no primeiro erro

    try {
        oxente::Scanner scanner(fonte.get());
        const auto programa = oxente::parse(scanner, &engine);
        if (!programa) return 1;

        // Toda saída é preparada antes de ser escrita, para que um erro não deixe
        // resultado parcial nem altere um arquivo de saída já existente.
        if (modo == "--dump-ast") {
            std::ostringstream texto;
            oxente::AstPrinter impressora(texto);
            programa->accept(impressora);
            std::cout << texto.str();
            return 0;
        }

        oxente::SemanticAnalyzer analise(&engine);
        programa->accept(analise);
        if (analise.tem_erros) return 1;

        if (modo == "--check") {
            std::cout << entrada << ": programa valido\n";
            return 0;
        }

        oxente::PythonGenerator gerador;
        const std::string codigo = gerador.gerar(*programa, oxente::RUNTIME_PYTHON);
        if (!tem_saida) {
            std::cout << codigo;
            return 0;
        }
        std::ofstream arquivo_saida(caminho_saida, std::ios::binary);
        arquivo_saida << codigo;
        if (!arquivo_saida.flush()) return erro("não foi possível escrever a saída: " + caminho_saida, 3);
        return 0;

    } catch (const oxente::ErroGeracao& falha) {
        return erro(std::string("erro interno do tradutor: ") + falha.what(), 1);

    } catch (const oxente::ErroLexico& falha) {
        const oxente::SourceLocation local{falha.local.first_line, falha.local.first_column};
        std::cerr << oxente::Diagnostic(oxente::Stage::Lexico, local, "erro_lexico", falha.what(), entrada).formatar()
                  << '\n';
        return 1;

    } catch (const std::out_of_range&) {
        // std::stoi/std::stod na construção dos literais; os limites da linguagem estão pendentes.
        return erro("literal numérico fora da faixa suportada.", 1);

    } catch (const oxente::DiagnosticException&) {
        // O motor já escreveu o diagnóstico; nenhum Python foi emitido.
        return 1;
    }
}
