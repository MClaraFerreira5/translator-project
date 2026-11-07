#pragma once

#include <iostream>
#include "Diagnostic.hpp"

namespace oxente {

class DiagnosticEngine {
private:
    std::string arquivo_;
    bool tem_erro_{false};
    bool parar_no_primeiro_erro_{true};

public:
    explicit DiagnosticEngine(std::string arquivo = "", bool parar_no_primeiro = true)
        : arquivo_(std::move(arquivo)), parar_no_primeiro_erro_(parar_no_primeiro) {}

    void emitir(Stage etapa, SourceLocation loc, const std::string& causa, const std::string& msg) {
        tem_erro_ = true;
        Diagnostic diag(etapa, loc, causa, msg, arquivo_);
        std::cerr << diag.formatar() << std::endl;

        if (parar_no_primeiro_erro_) {
            throw DiagnosticException(std::move(diag));
        }
    }

    // Helpers com as causas clássicas da especificação Oxente Lang:
    void erro_variavel_nao_declarada(SourceLocation loc, const std::string& nome) {
        emitir(Stage::Semantico, loc, "nome_nao_declarado",
               "Tais bestando? Variavel '" + nome + "' nao declarada.");
    }

    void erro_misturou_as_coisas(SourceLocation loc, const std::string& esperado, const std::string& obtido) {
        emitir(Stage::Semantico, loc, "misturou_as_coisas",
               "Incompatibilidade de tipos: esperado '" + esperado + "', mas obteve '" + obtido + "'.");
    }

    void erro_nome_duplicado(SourceLocation loc, const std::string& nome) {
        emitir(Stage::Semantico, loc, "valor_sem_pe_nem_cabeca",
               "Identificador '" + nome + "' ja declarado neste escopo.");
    }

    void erro_divisao_por_zero(SourceLocation loc) {
        emitir(Stage::Execucao, loc, "dividiu_por_nada",
               "Tentativa de divisao ou resto por zero.");
    }

    bool tem_erros() const { return tem_erro_; }
};

} // namespace oxente