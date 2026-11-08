// Testes de ponta a ponta a partir de código .oxe: parser -> AST -> visualização
// -> escopos -> gerador -> execução com Python 3, sempre sobre a mesma AST.
//
// Uso: teste-pipeline <python> <raiz-do-projeto> <dir-temporario>

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "ast/AstPrinter.hpp"
#include "codegen/PythonGenerator.hpp"
#include "frontend/scanner.hpp"
#include "semantic/ScopeVisitor.hpp"

namespace {

namespace fs = std::filesystem;

std::string python_exe;
fs::path raiz, temporario;
int casos = 0, falhas = 0, sequencia = 0;

struct FecharArquivo {
    void operator()(FILE* arquivo) const { std::fclose(arquivo); }
};

std::string ler(const fs::path& caminho) {
    std::ifstream arquivo(caminho, std::ios::binary);
    std::string bruto((std::istreambuf_iterator<char>(arquivo)), std::istreambuf_iterator<char>());
    std::string texto;
    for (char c : bruto) {
        if (c != '\r') texto += c;
    }
    return texto;
}

std::unique_ptr<ProgramNode> analisar(const std::string& codigo) {
    std::unique_ptr<FILE, FecharArquivo> arquivo(std::tmpfile());
    if (!arquivo) throw std::runtime_error("falha ao criar arquivo temporario");
    std::fwrite(codigo.data(), 1, codigo.size(), arquivo.get());
    std::rewind(arquivo.get());
    oxente::Scanner scanner(arquivo.get());
    oxente::DiagnosticEngine engine("teste.oxe", true);
    return oxente::parse(scanner, &engine);
}

std::string visualizar(ProgramNode& programa) {
    std::ostringstream texto;
    oxente::AstPrinter impressora(texto);
    programa.accept(impressora);
    return texto.str();
}

void falha(const std::string& nome, const std::string& detalhe) {
    ++falhas;
    std::cerr << "[FALHA] " << nome << ": " << detalhe << "\n";
}

void compara_ast(const std::string& nome, const std::string& codigo, const std::string& esperado) {
    ++casos;
    auto programa = analisar(codigo);
    if (!programa) return falha(nome, "parser rejeitou o programa");
    const std::string obtido = visualizar(*programa);
    if (obtido != esperado) falha(nome, "AST difere; obtida:\n" + obtido + "esperada:\n" + esperado);
}

std::string executar(const std::string& codigo_python, std::string& codigo_saida) {
    const fs::path base = temporario / ("programa" + std::to_string(++sequencia));
    const std::string py = base.string() + ".py", saida = base.string() + ".out";
    const fs::path runtime = raiz / "src" / "runtime";
    std::ofstream(py) << "import sys\nsys.path.insert(0, r\"" << runtime.string() << "\")\n" << codigo_python;

    std::string comando = "\"" + python_exe + "\" -B \"" + py + "\" > \"" + saida + "\" 2>&1";
#ifdef _WIN32
    comando = "\"" + comando + "\""; // cmd.exe remove o primeiro e o último par de aspas.
#endif
    codigo_saida = std::to_string(std::system(comando.c_str()));
    return ler(saida);
}

// Percorre a mesma AST com todas as etapas seguintes, sem reconstruí-la.
void executa(const std::string& nome, const std::string& codigo, const std::string& esperado) {
    ++casos;
    auto programa = analisar(codigo);
    if (!programa) return falha(nome, "parser rejeitou o programa");

    const std::string antes = visualizar(*programa);
    oxente::DiagnosticEngine engine(nome, true);
    oxente::ScopeVisitor escopos(&engine);
    programa->accept(escopos);
    oxente::PythonGenerator gerador;
    const std::string python = gerador.gerar(*programa);
    if (visualizar(*programa) != antes) return falha(nome, "as etapas alteraram a AST");

    std::string codigo_saida;
    const std::string saida = executar(python, codigo_saida);
    if (codigo_saida != "0") return falha(nome, "programa falhou:\n" + python + saida);
    if (saida != esperado) falha(nome, "saida difere:\n" + saida + "esperada:\n" + esperado + "\n" + python);
}

void executa_exemplo(const std::string& arquivo, const std::string& esperado) {
    executa(arquivo, ler(raiz / "examples" / "valid" / arquivo), esperado);
}

void rejeita_escopo(const std::string& arquivo) {
    ++casos;
    auto programa = analisar(ler(raiz / "examples" / "invalid" / arquivo));
    if (!programa) return falha(arquivo, "parser rejeitou o programa");
    try {
        oxente::DiagnosticEngine engine(arquivo, true);
        oxente::ScopeVisitor escopos(&engine);
        programa->accept(escopos);
        falha(arquivo, "analise de escopos aceitou o programa");
    } catch (const oxente::DiagnosticException&) {
    }
}

std::string ramos(int valor) {
    return "inteiro n bota " + std::to_string(valor) +
           "\nvai_que (n == 1) bora\n    desembucha(vei um tendeu)\narremata\n\n"
           "se_num_for (n == 2) bora\n    desembucha(vei dois tendeu)\narremata\n"
           "primeiramente comentario entre ramos\n"
           "se_num_for (n == 3) bora\n    desembucha(vei tres tendeu)\narremata\n"
           "pois_pronto bora\n    desembucha(vei outro tendeu)\narremata\n";
}

void testes_visualizacao() {
    compara_ast("precedencia", "inteiro r bota 2 + 3 * 4\n",
                "Programa\n  Bloco\n    Declaracao inteiro r\n      Binario +\n        Literal inteiro 2\n"
                "        Binario *\n          Literal inteiro 3\n          Literal inteiro 4\n");
    compara_ast("parenteses", "inteiro r bota (2 + 3) * 4\n",
                "Programa\n  Bloco\n    Declaracao inteiro r\n      Binario *\n        Binario +\n"
                "          Literal inteiro 2\n          Literal inteiro 3\n        Literal inteiro 4\n");
    compara_ast("comandos_simples", "tronxo t bota 7,5\nt bota abs(-t)\ndiga_ai(t)\ndesembucha(vei oi tendeu; evera)\n",
                "Programa\n  Bloco\n    Declaracao tronxo t\n      Literal tronxo 7,5\n    Atribuicao t\n"
                "      Unario abs\n        Unario -\n          Identificador t\n    DigaAi t\n    Desembucha\n"
                "      Literal resenha \"oi\"\n      Literal papo evera\n");
    compara_ast("condicional",
                "vai_que (a) bora\n    canso\narremata\nse_num_for (b) bora\narremata\npois_pronto bora\n    simbora\narremata\n",
                "Programa\n  Bloco\n    Se\n      Condicao\n        Identificador a\n      Entao\n        Bloco\n"
                "          Canso\n      Senao\n        Bloco\n          Se\n            Condicao\n"
                "              Identificador b\n            Entao\n              Bloco\n            Senao\n"
                "              Bloco\n                Simbora\n");
    compara_ast("num_para", "num_para ate (pala) bora\n    canso\narremata",
                "Programa\n  Bloco\n    NumPara\n      Parada\n        Literal papo pala\n      Corpo\n"
                "        Bloco\n          Canso\n");
    compara_ast("arrodeia", "arrodeia (inteiro i bota 0; ate i > 1; i bota 1) bora\narremata\n",
                "Programa\n  Bloco\n    Arrodeia\n      Inicio\n        Declaracao inteiro i\n"
                "          Literal inteiro 0\n      Parada\n        Binario >\n          Identificador i\n"
                "          Literal inteiro 1\n      Atualizacao\n        Atribuicao i\n"
                "          Literal inteiro 1\n      Corpo\n        Bloco\n");
}

void testes_exemplos() {
    executa_exemplo("calculo.oxe", "Resultado 14\nCom parenteses 20\n");
    executa_exemplo("decisao.oxe", "Acesso permitido\n");
    executa_exemplo("repeticao.oxe", "15\n");
    executa_exemplo("reais.oxe", "3,500\n2,500\n");
    executa_exemplo("ramos.oxe", "Recuperacao\n");
    executa_exemplo("comentarios.oxe", "primeiramente espia arriba\n");
    executa_exemplo("operadores-longos.oxe", "3\n");
    executa_exemplo("aninhamento.oxe", "32\n");
    executa_exemplo("escopo-independente.oxe", "10\n20\n");
    executa_exemplo("lacos-aninhados.oxe", "4\n");
    executa_exemplo("nl-crlf-eof.oxe", "3\n");
    executa_exemplo("precedencia-logica.oxe", "evera\npala\nevera\n");
    executa_exemplo("precedencia-potencia.oxe", "512,000\n-4,000\n24,000\n");
    rejeita_escopo("semantico-escopo-fechado.oxe");
    rejeita_escopo("semantico-sombreamento.oxe");
}

// Critérios da OX-12 verificados a partir de código-fonte.
void testes_fluxo() {
    executa("ramo_vai_que", ramos(1), "um\n");
    executa("ramo_se_num_for", ramos(2), "dois\n");
    executa("ramo_se_num_for_2", ramos(3), "tres\n");
    executa("ramo_pois_pronto", ramos(9), "outro\n");
    executa("sem_quebra_final", "vai_que (evera) bora\n    desembucha(1)\narremata", "1\n");
    executa("crlf", "vai_que (evera) bora\r\n    desembucha(1)\r\narremata\r\n", "1\n");
    executa("bloco_vazio", "vai_que (evera) bora\narremata\npois_pronto bora\n\narremata\ndesembucha(1)\n", "1\n");

    executa("num_para_parada_inicial", "num_para ate (evera) bora\n    desembucha(0)\narremata\ndesembucha(1)\n", "1\n");
    executa("arrodeia_parada_inicial",
            "arrodeia (inteiro i bota 9; ate i > 4; i bota i + 1) bora\n    desembucha(0)\narremata\ndesembucha(1)\n", "1\n");
    executa("arrodeia_simbora",
            "arrodeia (inteiro i bota 1; ate i > 4; i bota i + 1) bora\n    vai_que (i % 2 == 0) bora\n        simbora\n"
            "    arremata\n    desembucha(i)\narremata\n",
            "1\n3\n");
    executa("arrodeia_canso_sem_atualizar",
            "inteiro i bota 0\narrodeia (i bota 1; ate i > 9; i bota i + 1) bora\n    vai_que (i == 3) bora\n        canso\n"
            "    arremata\narremata\ndesembucha(i)\n",
            "3\n");
    executa("canso_encerra_laco_interno",
            "arrodeia (inteiro i bota 1; ate i > 2; i bota i + 1) bora\n    num_para ate (pala) bora\n        canso\n"
            "    arremata\n    desembucha(i)\narremata\n",
            "1\n2\n");
    executa("simbora_em_laco_interno",
            "arrodeia (inteiro i bota 1; ate i > 2; i bota i + 1) bora\n"
            "    arrodeia (inteiro j bota 1; ate j > 3; j bota j + 1) bora\n        vai_que (j == 2) bora\n            simbora\n"
            "        arremata\n        desembucha(i; j)\n    arremata\narremata\n",
            "1 1\n1 3\n2 1\n2 3\n");
    executa("escopos_independentes",
            "vai_que (evera) bora\n    inteiro x bota 1\n    desembucha(x)\narremata\n"
            "vai_que (evera) bora\n    resenha x bota vei oxe tendeu\n    desembucha(x)\narremata\n"
            "arrodeia (tronxo x bota 2; ate x > 2; x bota x + 1) bora\n    desembucha(x)\narremata\n",
            "1\noxe\n2,000\n");
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Uso: teste-pipeline <python> <raiz-do-projeto> <dir-temporario>\n";
        return 2;
    }
    python_exe = argv[1];
    raiz = argv[2];
    temporario = argv[3];
    fs::create_directories(temporario);

    try {
        testes_visualizacao();
        testes_exemplos();
        testes_fluxo();
    } catch (const std::exception& erro) {
        std::cerr << "[FALHA] excecao inesperada: " << erro.what() << "\n";
        return 1;
    }

    std::cout << casos - falhas << "/" << casos << " casos sem falha\n";
    return falhas == 0 ? 0 : 1;
}
