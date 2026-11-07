#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "common/Diagnostic.hpp"
#include "common/DiagnosticEngine.hpp"

namespace {
int erro(const std::string& mensagem, int codigo = 2) {
    std::cerr << "oxente: " << mensagem << '\n';
    return codigo;
}
void ajuda() {
    std::cout << "Uso: oxente MODO arquivo.oxe [-o caminho]\n"
                 "  --help                 Exibe esta ajuda (usado sozinho).\n"
                 "  --check                Validação: não implementado.\n"
                 "  --dump-ast             Visualização da AST: não implementado.\n"
                 "  --emit-python          Tradução Python 3: não implementado.\n"
                 "  -o, --output caminho   Saída exclusiva de --emit-python.\n"
                 "Exija um único modo e um arquivo .oxe; não há modo padrão.\n"
                 "Use -- antes de caminhos iniciados por hífen.\n"
                 "Os modos pendentes falham sem gerar arquivos.\n"
                 "Códigos: 0 ajuda; 2 argumentos; 3 entrada; 4 não implementado.\n";
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
    std::ifstream arquivo(caminho, std::ios::binary);
    if (!arquivo) return erro("não foi possível abrir a entrada: " + entrada, 3);
    // Não invocar o parser provisório nem abrir/truncar qualquer saída.
    // Prepara o motor de diagnóstico para o arquivo atual
    oxente::DiagnosticEngine engine(entrada, true); // true = para no primeiro erro

    try {
        // Quando as etapas forem conectadas, elas serão chamadas aqui dentro:
        // ex: auto ast = parser.executar(engine);
        // ex: semantico.verificar(ast, engine);
        // ex: codegen.gerar(ast, caminho_saida);

        return erro(modo + ": não implementado; nenhum programa foi analisado ou traduzido.", 4);

    } catch (const oxente::DiagnosticException& diag) {
        // CRITÉRIO DE ACEITE: "Garantir que erro impeça a emissão de Python válido"
        // Se houve erro e um arquivo de saída já tinha sido criado, apague-o!
        if (tem_saida && !caminho_saida.empty() && std::filesystem::exists(caminho_saida)) {
            std::filesystem::remove(caminho_saida);
        }

        // Retorna 1 = Erro de compilação/tradução
        return 1;
    }
}
