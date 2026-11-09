#include "PythonGenerator.hpp"

#include <charconv>
#include <cmath>
#include <cstdio>
#include <sstream>
#include <variant>

namespace oxente {

namespace {

// Níveis de precedência do Python, do menor para o maior. Potência, divisões e
// resto viram chamadas de helper e, portanto, são átomos.
enum Precedencia {
    PrecOu = 1,
    PrecE,
    PrecNao,
    PrecComparacao,
    PrecSoma,
    PrecMultiplicacao,
    PrecUnario,
    PrecAtomo
};

const char* const CABECALHO = "# Gerado pelo tradutor Oxente Lang. Nao edite manualmente.\n";
const char* const IMPORTACAO =
    "from oxente_runtime import _ox_desembucha, _ox_diga_ai, _ox_div, _ox_div_int, _ox_mod, _ox_pot, _ox_tronxo, _ox_soma, _ox_sub, _ox_mul\n";

template <typename Ponteiro>
Ponteiro* exigir(Ponteiro* ponteiro, const char* descricao) {
    if (!ponteiro) throw ErroGeracao(std::string("AST invalida: falta ") + descricao);
    return ponteiro;
}

std::string entreParenteses(const std::string& codigo, bool necessario) {
    return necessario ? "(" + codigo + ")" : codigo;
}

std::string literalInteiro(int valor) {
    return std::to_string(valor);
}

std::string literalReal(double valor) {
    if (!std::isfinite(valor)) throw ErroGeracao("AST invalida: literal real nao finito");
    char buffer[64];
    const auto fim = std::to_chars(buffer, buffer + sizeof buffer, valor).ptr;
    std::string texto(buffer, fim);
    // to_chars escreve 5.0 como "5", que seria um int em Python.
    if (texto.find_first_of(".eE") == std::string::npos) texto += ".0";
    return texto;
}

std::string literalTexto(const std::string& valor) {
    std::string texto = "\"";
    for (const unsigned char c : valor) {
        switch (c) {
        case '\\': texto += "\\\\"; break;
        case '"': texto += "\\\""; break;
        case '\n': texto += "\\n"; break;
        case '\r': texto += "\\r"; break;
        case '\t': texto += "\\t"; break;
        default:
            if (c < 0x20 || c >= 0x7f) {
                char escape[5];
                std::snprintf(escape, sizeof escape, "\\x%02x", static_cast<unsigned>(c));
                texto += escape;
            } else {
                texto += static_cast<char>(c);
            }
        }
    }
    return texto + "\"";
}

const char* nomeTipo(TypeKind tipo) {
    switch (tipo) {
    case TypeKind::Inteiro: return "inteiro";
    case TypeKind::Tronxo: return "tronxo";
    case TypeKind::Papo: return "papo";
    case TypeKind::Resenha: return "resenha";
    default: throw ErroGeracao("AST invalida: tipo de variavel nao suportado");
    }
}

TypeKind tipoAritmetico(TypeKind esquerda, TypeKind direita) {
    return esquerda == TypeKind::Inteiro && direita == TypeKind::Inteiro ? TypeKind::Inteiro
                                                                         : TypeKind::Tronxo;
}

} // namespace

std::string PythonGenerator::gerar(ProgramNode& programa, const std::string& runtime_embutido) {
    saida_ = CABECALHO;
    saida_ += runtime_embutido.empty() ? IMPORTACAO : runtime_embutido;
    saida_ += "\n\n";
    nivel_ = 0;
    escopos_.clear();
    ocorrencias_.clear();
    lacos_.clear();
    programa.accept(*this);
    return saida_;
}

void PythonGenerator::linha(const std::string& texto) {
    saida_.append(static_cast<std::size_t>(nivel_) * 4, ' ');
    saida_ += texto;
    saida_ += '\n';
}

PythonGenerator::Expressao PythonGenerator::avaliar(ExprNode* expressao, const char* descricao) {
    exigir(expressao, descricao)->accept(*this);
    return resultado_;
}

std::string PythonGenerator::avaliarConvertido(ExprNode* expressao, TypeKind destino,
                                               const char* descricao) {
    const Expressao valor = avaliar(expressao, descricao);
    // Promoção inteiro -> tronxo: sem conversão, a variável guardaria um int
    // Python e seria exibida sem as três casas decimais.
    if (destino == TypeKind::Tronxo && valor.tipo == TypeKind::Inteiro) {
        return "_ox_tronxo(" + valor.codigo + ", " + std::to_string(expressao->loc.line) + ")";
    }
    return valor.codigo;
}

PythonGenerator::Expressao PythonGenerator::chamadaRuntime(const char* helper, const Expressao& esquerda,
                                                           const Expressao& direita, int linha_origem,
                                                           TypeKind tipo) const {
    return {std::string(helper) + "(" + esquerda.codigo + ", " + direita.codigo + ", " +
                std::to_string(linha_origem) + ")",
            PrecAtomo, tipo};
}

const PythonGenerator::Variavel& PythonGenerator::declarar(const std::string& nome, TypeKind tipo) {
    const std::string interno = "v" + std::to_string(++ocorrencias_[nome]) + "_" + nome;
    return escopos_.back()[nome] = Variavel{interno, tipo};
}

const PythonGenerator::Variavel& PythonGenerator::buscar(const std::string& nome) const {
    for (auto escopo = escopos_.rbegin(); escopo != escopos_.rend(); ++escopo) {
        const auto encontrado = escopo->find(nome);
        if (encontrado != escopo->end()) return encontrado->second;
    }
    throw ErroGeracao("AST invalida: nome nao declarado: " + nome);
}

void PythonGenerator::emitirCorpo(BlockStmtNode* corpo, const Linhas& apos_corpo) {
    exigir(corpo, "corpo de bloco");
    ++nivel_;
    const std::size_t inicio = saida_.size();
    corpo->accept(*this);
    for (const auto& texto : apos_corpo) linha(texto);
    if (saida_.size() == inicio) linha("pass");
    --nivel_;
}

PythonGenerator::Linhas PythonGenerator::capturar(StmtNode& comando) {
    std::string externa;
    externa.swap(saida_);
    const int nivel_externo = nivel_;
    nivel_ = 0;
    comando.accept(*this);
    nivel_ = nivel_externo;
    externa.swap(saida_);

    // Textos têm quebras escapadas, então cada \n real separa uma linha Python.
    Linhas linhas;
    std::istringstream leitor(externa);
    for (std::string atual; std::getline(leitor, atual);) linhas.push_back(atual);
    return linhas;
}

void PythonGenerator::visit(ProgramNode& node) {
    exigir(node.bloco_principal.get(), "bloco principal")->accept(*this);
}

void PythonGenerator::visit(BlockStmtNode& node) {
    escopos_.emplace_back();
    for (auto& comando : node.comandos) {
        exigir(comando.get(), "comando do bloco")->accept(*this);
    }
    escopos_.pop_back();
}

void PythonGenerator::visit(VarDeclStmtNode& node) {
    // O inicializador é traduzido antes de o nome novo ficar visível.
    const std::string valor = avaliarConvertido(node.inicializador.get(), node.tipo, "inicializador");
    linha(declarar(node.nome, node.tipo).nome_python + " = " + valor);
}

void PythonGenerator::visit(AssignStmtNode& node) {
    const Variavel variavel = buscar(node.nome);
    linha(variavel.nome_python + " = " +
          avaliarConvertido(node.expressao.get(), variavel.tipo, "valor da atribuicao"));
}

void PythonGenerator::visit(OutputStmtNode& node) {
    std::string argumentos;
    for (auto& argumento : node.argumentos) {
        if (!argumentos.empty()) argumentos += ", ";
        argumentos += avaliar(argumento.get(), "argumento de desembucha").codigo;
    }
    linha("_ox_desembucha(" + argumentos + ")");
}

void PythonGenerator::visit(InputStmtNode& node) {
    const Variavel variavel = buscar(node.nome_variavel);
    linha(variavel.nome_python + " = _ox_diga_ai(\"" + nomeTipo(variavel.tipo) + "\", " +
          std::to_string(node.loc.line) + ")");
}

void PythonGenerator::visit(IfStmtNode& node) {
    IfStmtNode* ramo = &node;
    const char* palavra = "if";
    while (true) {
        linha(std::string(palavra) + " " + avaliar(ramo->condicao.get(), "condicao").codigo + ":");
        emitirCorpo(ramo->entao_bloco.get());

        BlockStmtNode* senao = ramo->senao_bloco.get();
        if (!senao) return;

        IfStmtNode* proximo =
            senao->comandos.size() == 1 ? dynamic_cast<IfStmtNode*>(senao->comandos[0].get()) : nullptr;
        if (!proximo) {
            linha("else:");
            emitirCorpo(senao);
            return;
        }
        ramo = proximo;
        palavra = "elif";
    }
}

void PythonGenerator::visit(WhileStmtNode& node) {
    linha("while not (" + avaliar(node.condicao.get(), "condicao de parada").codigo + "):");
    lacos_.push_back(nullptr);
    emitirCorpo(node.corpo.get());
    lacos_.pop_back();
}

void PythonGenerator::visit(ForStmtNode& node) {
    // A inicialização pertence a um escopo que envolve o laço inteiro.
    escopos_.emplace_back();
    if (node.inicializacao) node.inicializacao->accept(*this);

    const std::string parada = avaliar(node.condicao.get(), "condicao de parada").codigo;
    const Linhas atualizacao = node.incremento ? capturar(*node.incremento) : Linhas{};

    linha("while not (" + parada + "):");
    lacos_.push_back(&atualizacao);
    emitirCorpo(node.corpo.get(), atualizacao);
    lacos_.pop_back();
    escopos_.pop_back();
}

void PythonGenerator::visit(BreakStmtNode&) {
    if (lacos_.empty()) throw ErroGeracao("AST invalida: canso fora de laco");
    linha("break");
}

void PythonGenerator::visit(ContinueStmtNode&) {
    if (lacos_.empty()) throw ErroGeracao("AST invalida: simbora fora de laco");
    // O continue de Python pularia a atualização que fica no fim do corpo.
    if (const Linhas* atualizacao = lacos_.back()) {
        for (const auto& texto : *atualizacao) linha(texto);
    }
    linha("continue");
}

void PythonGenerator::visit(LiteralExprNode& node) {
    if (const int* inteiro = std::get_if<int>(&node.valor)) {
        resultado_ = {literalInteiro(*inteiro), *inteiro < 0 ? PrecUnario : PrecAtomo, TypeKind::Inteiro};
    } else if (const double* real = std::get_if<double>(&node.valor)) {
        resultado_ = {literalReal(*real), std::signbit(*real) ? PrecUnario : PrecAtomo, TypeKind::Tronxo};
    } else if (const bool* papo = std::get_if<bool>(&node.valor)) {
        resultado_ = {*papo ? "True" : "False", PrecAtomo, TypeKind::Papo};
    } else {
        resultado_ = {literalTexto(std::get<std::string>(node.valor)), PrecAtomo, TypeKind::Resenha};
    }
}

void PythonGenerator::visit(IdentifierExprNode& node) {
    const Variavel& variavel = buscar(node.nome);
    resultado_ = {variavel.nome_python, PrecAtomo, variavel.tipo};
}

void PythonGenerator::visit(UnaryExprNode& node) {
    const Expressao operando = avaliar(node.operando.get(), "operando unario");
    switch (node.op) {
    case UnaryOp::Positivo:
        resultado_ = {"+" + entreParenteses(operando.codigo, operando.precedencia < PrecUnario),
                      PrecUnario, operando.tipo};
        return;
    case UnaryOp::Negativo:
        resultado_ = {"-" + entreParenteses(operando.codigo, operando.precedencia < PrecUnario),
                      PrecUnario, operando.tipo};
        return;
    case UnaryOp::Nao:
        resultado_ = {"not " + entreParenteses(operando.codigo, operando.precedencia < PrecNao),
                      PrecNao, TypeKind::Papo};
        return;
    case UnaryOp::Abs:
        resultado_ = {"abs(" + operando.codigo + ")", PrecAtomo, operando.tipo};
        return;
    }
    throw ErroGeracao("AST invalida: operador unario desconhecido");
}

void PythonGenerator::visit(BinaryExprNode& node) {
    const Expressao esquerda = avaliar(node.esquerda.get(), "operando esquerdo");
    const Expressao direita = avaliar(node.direita.get(), "operando direito");
    const int linha_origem = node.loc.line;

    const char* simbolo = nullptr;
    int precedencia = 0;
    TypeKind tipo = TypeKind::Papo;

    switch (node.op) {
    case BinaryOp::Divisao:
        resultado_ = chamadaRuntime("_ox_div", esquerda, direita, linha_origem, TypeKind::Tronxo);
        return;
    case BinaryOp::DivisaoInteira:
        resultado_ = chamadaRuntime("_ox_div_int", esquerda, direita, linha_origem, TypeKind::Inteiro);
        return;
    case BinaryOp::Modulo:
        resultado_ = chamadaRuntime("_ox_mod", esquerda, direita, linha_origem, TypeKind::Inteiro);
        return;
    case BinaryOp::Potencia:
        resultado_ = chamadaRuntime("_ox_pot", esquerda, direita, linha_origem, TypeKind::Tronxo);
        return;
    case BinaryOp::Soma: simbolo = "+"; precedencia = PrecSoma; break;
    case BinaryOp::Subtracao: simbolo = "-"; precedencia = PrecSoma; break;
    case BinaryOp::Multiplicacao: simbolo = "*"; precedencia = PrecMultiplicacao; break;
    case BinaryOp::Igual: simbolo = "=="; precedencia = PrecComparacao; break;
    case BinaryOp::Diferente: simbolo = "!="; precedencia = PrecComparacao; break;
    case BinaryOp::Menor: simbolo = "<"; precedencia = PrecComparacao; break;
    case BinaryOp::MenorIgual: simbolo = "<="; precedencia = PrecComparacao; break;
    case BinaryOp::Maior: simbolo = ">"; precedencia = PrecComparacao; break;
    case BinaryOp::MaiorIgual: simbolo = ">="; precedencia = PrecComparacao; break;
    case BinaryOp::E: simbolo = "and"; precedencia = PrecE; break;
    case BinaryOp::Ou: simbolo = "or"; precedencia = PrecOu; break;
    }
    if (!simbolo) throw ErroGeracao("AST invalida: operador binario desconhecido");

    if (precedencia == PrecSoma || precedencia == PrecMultiplicacao) {
        tipo = tipoAritmetico(esquerda.tipo, direita.tipo);
        if (tipo == TypeKind::Tronxo) {
            const char* helper = node.op == BinaryOp::Soma ? "_ox_soma"
                                : node.op == BinaryOp::Subtracao ? "_ox_sub" : "_ox_mul";
            resultado_ = chamadaRuntime(helper, esquerda, direita, linha_origem, tipo);
            return;
        }
    }

    // Comparações em Python encadeiam (a < b < c), então um operando que também
    // seja comparação precisa de parênteses dos dois lados. Nos demais casos a
    // associação é à esquerda e só o operando direito de mesmo nível os exige.
    const bool protege_esquerda = precedencia == PrecComparacao ? esquerda.precedencia <= precedencia
                                                                : esquerda.precedencia < precedencia;
    resultado_ = {entreParenteses(esquerda.codigo, protege_esquerda) + " " + simbolo + " " +
                      entreParenteses(direita.codigo, direita.precedencia <= precedencia),
                  precedencia, tipo};
}

} // namespace oxente
