#pragma once

#include <string>
#include "common/TypeKind.hpp"
#include "common/SourceLocation.hpp"

namespace oxente {

struct Symbol {
    std::string nome;
    TypeKind tipo{TypeKind::Indefinido};
    SourceLocation loc{0, 0};
    int nivel_escopo{0};
    bool inicializado{false};

    Symbol() = default;

    Symbol(std::string n, TypeKind t, SourceLocation l, int nivel, bool init = true)
        : nome(std::move(n)), tipo(t), loc(l), nivel_escopo(nivel), inicializado(init) {}
};

} // namespace oxente