#include "AstPrinter.hpp"

#include <charconv>
#include <variant>

namespace oxente {

namespace {

const char* nomeTipo(TypeKind tipo) {
    switch (tipo) {
    case TypeKind::Inteiro: return "inteiro";
    case TypeKind::Tronxo: return "tronxo";
    case TypeKind::Papo: return "papo";
    case TypeKind::Resenha: return "resenha";
    default: return "?";
    }
}

const char* simbolo(BinaryOp op) {
    switch (op) {
    case BinaryOp::Soma: return "+";
    case BinaryOp::Subtracao: return "-";
    case BinaryOp::Multiplicacao: return "*";
    case BinaryOp::Divisao: return "/";
    case BinaryOp::DivisaoInteira: return "//";
    case BinaryOp::Modulo: return "%";
    case BinaryOp::Potencia: return "**";
    case BinaryOp::Igual: return "==";
    case BinaryOp::Diferente: return "!=";
    case BinaryOp::Menor: return "<";
    case BinaryOp::MenorIgual: return "<=";
    case BinaryOp::Maior: return ">";
    case BinaryOp::MaiorIgual: return ">=";
    case BinaryOp::E: return "e";
    case BinaryOp::Ou: return "ou";
    }
    return "?";
}

const char* simbolo(UnaryOp op) {
    switch (op) {
    case UnaryOp::Positivo: return "+";
    case UnaryOp::Negativo: return "-";
    case UnaryOp::Nao: return "ai_dento";
    case UnaryOp::Abs: return "abs";
    }
    return "?";
}

std::string textoReal(double valor) {
    char buffer[64];
    const auto fim = std::to_chars(buffer, buffer + sizeof buffer, valor).ptr;
    std::string texto(buffer, fim);
    const auto ponto = texto.find('.');
    if (ponto == std::string::npos) return texto + ",0";
    texto[ponto] = ',';
    return texto;
}

} // namespace

void AstPrinter::linha(const std::string& texto) {
    saida_ << std::string(static_cast<std::size_t>(nivel_) * 2, ' ') << texto << '\n';
}

void AstPrinter::filho(AstNode* no) {
    if (!no) return;
    ++nivel_;
    no->accept(*this);
    --nivel_;
}

void AstPrinter::secao(const char* rotulo, AstNode* no) {
    if (!no) return;
    ++nivel_;
    linha(rotulo);
    filho(no);
    --nivel_;
}

void AstPrinter::visit(ProgramNode& node) {
    linha("Programa");
    filho(node.bloco_principal.get());
}

void AstPrinter::visit(BlockStmtNode& node) {
    linha("Bloco");
    for (auto& comando : node.comandos) filho(comando.get());
}

void AstPrinter::visit(VarDeclStmtNode& node) {
    linha(std::string("Declaracao ") + nomeTipo(node.tipo) + " " + node.nome);
    filho(node.inicializador.get());
}

void AstPrinter::visit(AssignStmtNode& node) {
    linha("Atribuicao " + node.nome);
    filho(node.expressao.get());
}

void AstPrinter::visit(OutputStmtNode& node) {
    linha("Desembucha");
    for (auto& argumento : node.argumentos) filho(argumento.get());
}

void AstPrinter::visit(InputStmtNode& node) {
    linha("DigaAi " + node.nome_variavel);
}

void AstPrinter::visit(IfStmtNode& node) {
    linha("Se");
    secao("Condicao", node.condicao.get());
    secao("Entao", node.entao_bloco.get());
    secao("Senao", node.senao_bloco.get());
}

void AstPrinter::visit(WhileStmtNode& node) {
    linha("NumPara");
    secao("Parada", node.condicao.get());
    secao("Corpo", node.corpo.get());
}

void AstPrinter::visit(ForStmtNode& node) {
    linha("Arrodeia");
    secao("Inicio", node.inicializacao.get());
    secao("Parada", node.condicao.get());
    secao("Atualizacao", node.incremento.get());
    secao("Corpo", node.corpo.get());
}

void AstPrinter::visit(BreakStmtNode&) { linha("Canso"); }

void AstPrinter::visit(ContinueStmtNode&) { linha("Simbora"); }

void AstPrinter::visit(LiteralExprNode& node) {
    if (const int* inteiro = std::get_if<int>(&node.valor)) {
        linha("Literal inteiro " + std::to_string(*inteiro));
    } else if (const double* real = std::get_if<double>(&node.valor)) {
        linha("Literal tronxo " + textoReal(*real));
    } else if (const bool* papo = std::get_if<bool>(&node.valor)) {
        linha(std::string("Literal papo ") + (*papo ? "evera" : "pala"));
    } else {
        linha("Literal resenha \"" + std::get<std::string>(node.valor) + "\"");
    }
}

void AstPrinter::visit(IdentifierExprNode& node) {
    linha("Identificador " + node.nome);
}

void AstPrinter::visit(UnaryExprNode& node) {
    linha(std::string("Unario ") + simbolo(node.op));
    filho(node.operando.get());
}

void AstPrinter::visit(BinaryExprNode& node) {
    linha(std::string("Binario ") + simbolo(node.op));
    filho(node.esquerda.get());
    filho(node.direita.get());
}

} // namespace oxente
