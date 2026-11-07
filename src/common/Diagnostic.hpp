#pragma once

#include <iostream>
#include <string>
#include <utility>
#include "SourceLocation.hpp"

namespace oxente {

enum class Stage {
    Lexico,
    Sintatico,
    Semantico,
    Execucao
};

inline const char* to_string(Stage etapa) {
    switch (etapa) {
        case Stage::Lexico: return "léxico";
        case Stage::Sintatico: return "sintático";
        case Stage::Semantico: return "semântico";
        case Stage::Execucao: return "execução";
    }
    return "desconhecido";
}

struct Diagnostic {
    Stage etapa;
    SourceLocation loc;
    std::string causa_tecnica; // ex: "misturou_as_coisas", "nome_nao_declarado"
    std::string mensagem;       // ex: "Tipos incompatíveis: esperado 'tronxo', obtido 'inteiro'."
    std::string arquivo;

    Diagnostic(Stage etapa, SourceLocation loc, std::string causa, std::string msg, std::string arq = "")
        : etapa(etapa), loc(loc), causa_tecnica(std::move(causa)), mensagem(std::move(msg)), arquivo(std::move(arq)) {}

    // Formata no padrão legível da CLI
    std::string formatar() const {
        std::string resultado = "oxente [" + std::string(to_string(etapa)) + "]: ";
        if (!arquivo.empty()) {
            resultado += arquivo + ":";
        }
        if (loc.line > 0) {
            resultado += std::to_string(loc.line) + ":" + std::to_string(loc.column) + ": ";
        }
        resultado += "erro '" + causa_tecnica + "': " + mensagem;
        return resultado;
    }
};

// Exceção lançada para abortar o processo imediatamente no primeiro erro
class DiagnosticException : public std::exception {
    Diagnostic diag_;
    std::string formatado_;
public:
    explicit DiagnosticException(Diagnostic diag)
        : diag_(std::move(diag)), formatado_(diag_.formatar()) {}

    const char* what() const noexcept override {
        return formatado_.c_str();
    }

    const Diagnostic& get() const noexcept {
        return diag_;
    }
};

} // namespace oxente