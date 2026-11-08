#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <optional>
#include "Symbol.hpp"
#include "common/DiagnosticEngine.hpp"

namespace oxente {

class SymbolTable {
private:
    // Pilha de escopos ativos: escopos_[0] é o global; o último é o mais interno
    std::vector<std::unordered_map<std::string, Symbol>> escopos_;
    DiagnosticEngine* engine_{nullptr};

public:
    explicit SymbolTable(DiagnosticEngine* engine = nullptr) : engine_(engine) {
        // Inicia com o escopo global (nível 0)
        entrar_escopo();
    }

    // Nível atual de profundidade (0 = global)
    int nivel_atual() const {
        return static_cast<int>(escopos_.size()) - 1;
    }

    // Abre um novo bloco de escopo
    void entrar_escopo() {
        escopos_.emplace_back();
    }

    // Fecha o bloco de escopo atual, descartando os nomes locais
    void sair_escopo() {
        if (escopos_.size() > 1) {
            escopos_.pop_back();
        }
    }

    // Verifica se o identificador já está visível em QUALQUER escopo ativo (impede sombreamento)
    bool esta_visivel(const std::string& nome) const {
        for (auto it = escopos_.rbegin(); it != escopos_.rend(); ++it) {
            if (it->find(nome) != it->end()) {
                return true;
            }
        }
        return false;
    }

    // Insere uma nova declaração. Rejeita se já estiver visível em qualquer nível.
    bool inserir(const std::string& nome, TypeKind tipo, SourceLocation loc, bool inicializado = true) {
        // Regra Oxente: Proibido sombreamento de identificadores já visíveis
        if (esta_visivel(nome)) {
            if (engine_) {
                engine_->erro_nome_duplicado(loc, nome);
            }
            return false;
        }

        Symbol s(nome, tipo, loc, nivel_atual(), inicializado);
        escopos_.back()[nome] = s;
        return true;
    }

    // Busca o símbolo do escopo mais interno para o mais externo
    const Symbol* buscar(const std::string& nome) const {
        for (auto it = escopos_.rbegin(); it != escopos_.rend(); ++it) {
            auto encontrado = it->find(nome);
            if (encontrado != it->end()) {
                return &(encontrado->second);
            }
        }
        return nullptr;
    }

    // Valida o uso de uma variável: deve estar declarada e inicializada
    const Symbol* validar_uso(const std::string& nome, SourceLocation loc) {
        const Symbol* s = buscar(nome);
        if (!s) {
            if (engine_) {
                engine_->erro_variavel_nao_declarada(loc, nome);
            }
            return nullptr;
        }
        if (!s->inicializado) {
            if (engine_) {
                engine_->emitir(Stage::Semantico, loc, "variavel_nao_inicializada",
                                "Variavel '" + nome + "' declarada mas nao inicializada.");
            }
            return nullptr;
        }
        return s;
    }
};

} // namespace oxente