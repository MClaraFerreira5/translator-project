#include <cassert>
#include <iostream>
#include "../src/common/Diagnostic.hpp"
#include "../src/common/DiagnosticEngine.hpp"
#include "../src/common/SourceLocation.hpp"

void testar_formato_erro() {
    oxente::SourceLocation loc{10, 5};
    oxente::Diagnostic diag(
        oxente::Stage::Semantico,
        loc,
        "misturou_as_coisas",
        "Tipos incompativeis: esperado 'tronxo', obtido 'resenha'.",
        "programa.oxe"
    );

    std::string saida = diag.formatar();
    assert(saida.find("programa.oxe:10:5:") != std::string::npos);
    assert(saida.find("misturou_as_coisas") != std::string::npos);
    assert(saida.find("Tipos incompativeis") != std::string::npos);
    std::cout << "[OK] Formato do diagnostico validado com sucesso.\n";
}

void testar_interrupcao_no_primeiro_erro() {
    oxente::DiagnosticEngine engine("teste.oxe", true);
    bool capturou = false;

    try {
        engine.erro_variavel_nao_declarada({4, 2}, "quantidade");
    } catch (const oxente::DiagnosticException& e) {
        capturou = true;
        assert(std::string(e.get().causa_tecnica) == "nome_nao_declarado");
        assert(e.get().loc.line == 4);
    }

    assert(capturou);
    assert(engine.tem_erros());
    std::cout << "[OK] Interrupcao no primeiro erro validada com sucesso.\n";
}

int main() {
    testar_formato_erro();
    testar_interrupcao_no_primeiro_erro();
    std::cout << "\nTodos os testes de diagnosticos passaram!\n";
    return 0;
}