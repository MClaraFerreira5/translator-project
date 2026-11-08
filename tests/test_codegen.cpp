// Testes do gerador Python com ASTs montadas manualmente. Cada caso gera o
// programa, executa-o com Python 3 e compara a saída com o valor esperado.
//
// Uso: teste-codegen <python> <dir-runtime> <dir-temporario>

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "codegen/PythonGenerator.hpp"

namespace {

using E = std::unique_ptr<ExprNode>;
using S = std::unique_ptr<StmtNode>;
using B = std::unique_ptr<BlockStmtNode>;
using oxente::PythonGenerator;

std::string python_exe, dir_runtime;
std::filesystem::path dir_temporario;
int falhas = 0, casos = 0;

// --- Construção da AST ---------------------------------------------------

E inteiro(int v) { return std::make_unique<LiteralExprNode>(v, TypeKind::Inteiro); }
E real(double v) { return std::make_unique<LiteralExprNode>(v, TypeKind::Tronxo); }
E papo(bool v) { return std::make_unique<LiteralExprNode>(v, TypeKind::Papo); }
E texto(const std::string& v) { return std::make_unique<LiteralExprNode>(v, TypeKind::Resenha); }
E id(const std::string& nome) { return std::make_unique<IdentifierExprNode>(nome); }
E bin(BinaryOp op, E a, E b, int linha = 0) {
    return std::make_unique<BinaryExprNode>(op, std::move(a), std::move(b), SourceLocation{linha, 1});
}
E un(UnaryOp op, E a) { return std::make_unique<UnaryExprNode>(op, std::move(a)); }

E soma(E a, E b) { return bin(BinaryOp::Soma, std::move(a), std::move(b)); }
E sub(E a, E b) { return bin(BinaryOp::Subtracao, std::move(a), std::move(b)); }
E mul(E a, E b) { return bin(BinaryOp::Multiplicacao, std::move(a), std::move(b)); }
E pot(E a, E b) { return bin(BinaryOp::Potencia, std::move(a), std::move(b)); }
E maior(E a, E b) { return bin(BinaryOp::Maior, std::move(a), std::move(b)); }
E igual(E a, E b) { return bin(BinaryOp::Igual, std::move(a), std::move(b)); }
E resto(E a, E b) { return bin(BinaryOp::Modulo, std::move(a), std::move(b)); }

S decl(TypeKind t, const std::string& nome, E valor) {
    return std::make_unique<VarDeclStmtNode>(t, nome, std::move(valor));
}
S inteiro_v(const std::string& nome, int valor) { return decl(TypeKind::Inteiro, nome, inteiro(valor)); }
S bota(const std::string& nome, E valor) { return std::make_unique<AssignStmtNode>(nome, std::move(valor)); }
S incrementa(const std::string& nome) { return bota(nome, soma(id(nome), inteiro(1))); }
S canso() { return std::make_unique<BreakStmtNode>(); }
S simbora() { return std::make_unique<ContinueStmtNode>(); }

template <typename... Args>
S saida(Args... args) {
    std::vector<E> argumentos;
    (argumentos.push_back(std::move(args)), ...);
    return std::make_unique<OutputStmtNode>(std::move(argumentos));
}

template <typename... Cmds>
B bloco(Cmds... cmds) {
    auto b = std::make_unique<BlockStmtNode>();
    (b->add(std::move(cmds)), ...);
    return b;
}

S se(E cond, B entao, B senao = nullptr) {
    return std::make_unique<IfStmtNode>(std::move(cond), std::move(entao), std::move(senao));
}
S num_para(E parada, B corpo) { return std::make_unique<WhileStmtNode>(std::move(parada), std::move(corpo)); }
S arrodeia(S inicio, E parada, S atualizacao, B corpo) {
    return std::make_unique<ForStmtNode>(std::move(inicio), std::move(parada), std::move(atualizacao),
                                         std::move(corpo));
}

// --- Execução ------------------------------------------------------------

std::string ler(const std::filesystem::path& caminho) {
    std::ifstream arquivo(caminho, std::ios::binary);
    std::string conteudo((std::istreambuf_iterator<char>(arquivo)), std::istreambuf_iterator<char>());
    std::string sem_cr;
    for (char c : conteudo) {
        if (c != '\r') sem_cr += c;
    }
    return sem_cr;
}

struct Execucao {
    std::string codigo, saida, erro;
    bool sucesso;
};

Execucao executar(const std::string& nome, B programa_corpo, const std::string& entrada) {
    ProgramNode programa(std::move(programa_corpo));
    PythonGenerator gerador;
    Execucao r;
    r.codigo = gerador.gerar(programa);

    const auto base = dir_temporario / nome;
    const auto py = base.string() + ".py";
    const auto arq_entrada = base.string() + ".in";
    const auto arq_saida = base.string() + ".out";
    const auto arq_erro = base.string() + ".err";

    std::ofstream(py) << "import sys\nsys.path.insert(0, r\"" << dir_runtime << "\")\n" << r.codigo;
    std::ofstream(arq_entrada) << entrada;

    std::string comando = "\"" + python_exe + "\" -B \"" + py + "\" < \"" + arq_entrada + "\" > \"" +
                          arq_saida + "\" 2> \"" + arq_erro + "\"";
#ifdef _WIN32
    comando = "\"" + comando + "\""; // cmd.exe remove o primeiro e o último par de aspas.
#endif
    r.sucesso = std::system(comando.c_str()) == 0;
    r.saida = ler(arq_saida);
    r.erro = ler(arq_erro);
    return r;
}

void falha(const std::string& nome, const std::string& motivo, const Execucao& r) {
    ++falhas;
    std::cerr << "[FALHA] " << nome << ": " << motivo << "\n--- codigo ---\n" << r.codigo
              << "--- stdout ---\n" << r.saida << "--- stderr ---\n" << r.erro << "\n";
}

void contem(const std::string& nome, const Execucao& r, const std::string& trecho) {
    if (r.codigo.find(trecho) == std::string::npos) falha(nome, "codigo sem o trecho: " + trecho, r);
}

void nao_contem(const std::string& nome, const Execucao& r, const std::string& trecho) {
    if (r.codigo.find(trecho) != std::string::npos) falha(nome, "codigo com o trecho: " + trecho, r);
}

Execucao caso(const std::string& nome, B corpo, const std::string& esperado, const std::string& entrada = "") {
    ++casos;
    Execucao r = executar(nome, std::move(corpo), entrada);
    if (!r.sucesso) falha(nome, "programa gerado terminou com erro", r);
    else if (r.saida != esperado) falha(nome, "saida difere; esperado:\n" + esperado, r);
    return r;
}

template <typename Construir>
void rejeita(const std::string& nome, Construir construir) {
    ++casos;
    try {
        ProgramNode programa(construir());
        PythonGenerator gerador;
        gerador.gerar(programa);
        ++falhas;
        std::cerr << "[FALHA] " << nome << ": AST invalida foi aceita\n";
    } catch (const oxente::ErroGeracao&) {
    }
}

// --- Atividade 12: expressões e comandos simples --------------------------

void testes_expressoes() {
    caso("literais",
         bloco(saida(inteiro(42)), saida(real(7.5)), saida(real(5.0)), saida(papo(true)), saida(papo(false)),
               saida(texto("aspas \" barra \\ e 'simples'"))),
         "42\n7,500\n5,000\nevera\npala\naspas \" barra \\ e 'simples'\n");

    // Exemplo 4.1 da proposta.
    caso("proposta_calculo",
         bloco(inteiro_v("x1", 2), inteiro_v("x2", 3), inteiro_v("x3", 4),
               decl(TypeKind::Inteiro, "resultado", soma(id("x1"), mul(id("x2"), id("x3")))),
               saida(texto("Resultado"), id("resultado")),
               decl(TypeKind::Inteiro, "agrupado", mul(soma(id("x1"), id("x2")), id("x3"))),
               saida(texto("Com parenteses"), id("agrupado"))),
         "Resultado 14\nCom parenteses 20\n");

    caso("associatividade",
         bloco(saida(sub(sub(inteiro(10), inteiro(3)), inteiro(2))),
               saida(sub(inteiro(10), sub(inteiro(3), inteiro(2)))),
               saida(mul(inteiro(2), soma(inteiro(3), inteiro(4)))),
               saida(sub(inteiro(1), un(UnaryOp::Negativo, inteiro(2)))),
               saida(un(UnaryOp::Negativo, soma(inteiro(1), inteiro(2))))),
         "5\n9\n14\n3\n-3\n");

    // 2 ** 3 ** 2 e -2 ** 2 da tabela 6.8 da proposta.
    caso("potencia",
         bloco(saida(pot(inteiro(2), pot(inteiro(3), inteiro(2)))),
               saida(pot(pot(inteiro(2), inteiro(3)), inteiro(2))),
               saida(un(UnaryOp::Negativo, pot(inteiro(2), inteiro(2)))),
               saida(pot(un(UnaryOp::Negativo, inteiro(2)), inteiro(2)))),
         "512,000\n64,000\n-4,000\n4,000\n");

    // Sem parênteses, Python encadearia a comparação ou rejeitaria "x == not y".
    Execucao comparacoes = caso(
        "comparacoes_aninhadas",
        bloco(saida(igual(maior(inteiro(1), inteiro(2)), papo(false))),
              saida(igual(papo(false), maior(inteiro(1), inteiro(2)))),
              saida(igual(un(UnaryOp::Nao, papo(false)), papo(true))),
              saida(igual(papo(true), un(UnaryOp::Nao, papo(false)))),
              saida(un(UnaryOp::Nao, igual(inteiro(1), inteiro(1))))),
        "evera\nevera\nevera\nevera\npala\n");
    contem("comparacoes_aninhadas", comparacoes, "(1 > 2) == False");

    // (a ou b) e c precisa manter o agrupamento.
    caso("logica",
         bloco(saida(bin(BinaryOp::E, bin(BinaryOp::Ou, papo(true), papo(false)), papo(false))),
               saida(bin(BinaryOp::Ou, papo(true), bin(BinaryOp::E, papo(false), papo(false)))),
               saida(bin(BinaryOp::Ou, papo(false), un(UnaryOp::Nao, papo(false))))),
         "pala\nevera\nevera\n");

    // O operando direito não é avaliado: a divisão por zero não pode ocorrer.
    caso("curto_circuito",
         bloco(saida(bin(BinaryOp::E, papo(false),
                         maior(bin(BinaryOp::Divisao, inteiro(1), inteiro(0)), inteiro(0))))),
         "pala\n");

    caso("abs_preserva_tipo",
         bloco(saida(un(UnaryOp::Abs, un(UnaryOp::Negativo, inteiro(3)))),
               saida(un(UnaryOp::Abs, un(UnaryOp::Negativo, real(2.5)))),
               saida(mul(un(UnaryOp::Abs, sub(inteiro(1), inteiro(4))), inteiro(2)))),
         "3\n2,500\n6\n");

    // Exemplo 4.4 da proposta.
    caso("proposta_reais",
         bloco(inteiro_v("quantidade", 3), decl(TypeKind::Tronxo, "total", id("quantidade")),
               decl(TypeKind::Tronxo, "metade", bin(BinaryOp::Divisao, inteiro(5), inteiro(2))),
               bota("total", soma(id("total"), real(0.5))), saida(id("total")), saida(id("metade"))),
         "3,500\n2,500\n");

    Execucao promocao = caso(
        "promocao_inteiro_tronxo",
        bloco(decl(TypeKind::Tronxo, "t", inteiro(3)), saida(id("t")), inteiro_v("i", 4), bota("t", id("i")),
              saida(id("t")), bota("t", soma(id("i"), inteiro(1))), saida(id("t")), saida(soma(id("i"), real(0.5))),
              saida(id("i"))),
        "3,000\n4,000\n5,000\n4,500\n4\n");
    contem("promocao_inteiro_tronxo", promocao, "float(");

    caso("divisao_inteira_e_resto",
         bloco(saida(bin(BinaryOp::DivisaoInteira, un(UnaryOp::Negativo, inteiro(7)), inteiro(2))),
               saida(resto(un(UnaryOp::Negativo, inteiro(7)), inteiro(2))),
               saida(bin(BinaryOp::DivisaoInteira, inteiro(7), inteiro(2))), saida(resto(inteiro(7), inteiro(2)))),
         "-3\n-1\n3\n1\n");

    ++casos;
    Execucao zero = executar("divisao_por_zero", bloco(saida(bin(BinaryOp::Divisao, inteiro(1), inteiro(0), 9))), "");
    if (zero.sucesso) falha("divisao_por_zero", "deveria falhar em execucao", zero);
    if (zero.erro.find("dividiu_por_nada") == std::string::npos || zero.erro.find("linha 9") == std::string::npos) {
        falha("divisao_por_zero", "diagnostico sem codigo ou linha", zero);
    }

    caso("saida_varios_argumentos",
         bloco(saida(inteiro(10), texto("pontos")), saida(texto("a"), real(1.5), papo(true), soma(inteiro(1), inteiro(1)))),
         "10 pontos\na 1,500 evera 2\n");

    caso("entrada",
         bloco(inteiro_v("i", 0), decl(TypeKind::Tronxo, "t", real(0.0)),
               decl(TypeKind::Papo, "p", papo(false)), decl(TypeKind::Resenha, "r", texto("")),
               std::make_unique<InputStmtNode>("i"), std::make_unique<InputStmtNode>("t"),
               std::make_unique<InputStmtNode>("p"), std::make_unique<InputStmtNode>("r"),
               saida(soma(id("i"), inteiro(1))), saida(id("t")), saida(id("p")), saida(id("r"))),
         "42\n7,500\nevera\noxente visse\n", "41\n7,5\nevera\noxente visse\n");

    // Palavras reservadas, builtins usados pelo código gerado e nomes de helpers.
    Execucao nomes = caso(
        "nomes_seguros",
        bloco(inteiro_v("class", 1), inteiro_v("print", 2), inteiro_v("float", 3), inteiro_v("None", 4),
              inteiro_v("_ox_desembucha", 5), inteiro_v("_ox_div", 6), inteiro_v("v1_class", 7),
              decl(TypeKind::Tronxo, "t", id("class")),
              saida(soma(id("class"), soma(id("print"), id("float"))), id("None"), id("_ox_desembucha")),
              saida(bin(BinaryOp::Divisao, id("_ox_div"), inteiro(2)), id("v1_class"), id("t"))),
        "6 4 5\n3,000 7 1,000\n");
    contem("nomes_seguros", nomes, "v1_class = 1");
    contem("nomes_seguros", nomes, "v1_v1_class = 7");
    nao_contem("nomes_seguros", nomes, "\nclass =");

    rejeita("nome_nao_declarado", [] { return bloco(saida(id("fantasma"))); });
    rejeita("uso_no_proprio_inicializador", [] { return bloco(decl(TypeKind::Inteiro, "x", soma(id("x"), inteiro(1)))); });
    rejeita("declaracao_sem_inicializador", [] { return bloco(decl(TypeKind::Inteiro, "x", nullptr)); });
}

// --- Atividade 13: fluxo de controle e escopos ---------------------------

B cadeia_ramos(int valor) {
    return bloco(
        inteiro_v("n", valor),
        se(igual(id("n"), inteiro(1)), bloco(saida(texto("um"))),
           bloco(se(igual(id("n"), inteiro(2)), bloco(saida(texto("dois"))),
                    bloco(se(igual(id("n"), inteiro(3)), bloco(saida(texto("tres"))), bloco(saida(texto("outro")))))))));
}

void testes_condicionais() {
    Execucao ramos = caso("ramo_vai_que", cadeia_ramos(1), "um\n");
    contem("ramo_vai_que", ramos, "if v1_n == 1:");
    contem("ramo_vai_que", ramos, "elif v1_n == 2:");
    contem("ramo_vai_que", ramos, "elif v1_n == 3:");
    contem("ramo_vai_que", ramos, "else:");
    caso("ramo_se_num_for", cadeia_ramos(2), "dois\n");
    caso("ramo_se_num_for_2", cadeia_ramos(3), "tres\n");
    caso("ramo_pois_pronto", cadeia_ramos(9), "outro\n");

    caso("vai_que_sem_senao",
         bloco(se(papo(false), bloco(saida(texto("nao")))), saida(texto("fim"))), "fim\n");

    // Um else com outros comandos além do if aninhado não pode virar elif.
    Execucao senao = caso("else_nao_vira_elif",
                          bloco(se(papo(false), bloco(saida(texto("a"))),
                                   bloco(saida(texto("b")), se(papo(true), bloco(saida(texto("c"))))))),
                          "b\nc\n");
    nao_contem("else_nao_vira_elif", senao, "elif");

    Execucao vazio = caso("blocos_vazios", bloco(se(papo(true), bloco(), bloco()), saida(texto("ok"))), "ok\n");
    contem("blocos_vazios", vazio, "pass");

    caso("proposta_decisao",
         bloco(inteiro_v("idade", 18), decl(TypeKind::Papo, "autorizado", papo(true)),
               decl(TypeKind::Papo, "bloqueado", papo(false)),
               se(bin(BinaryOp::E, bin(BinaryOp::Ou, bin(BinaryOp::MaiorIgual, id("idade"), inteiro(18)), id("autorizado")),
                      un(UnaryOp::Nao, id("bloqueado"))),
                  bloco(saida(texto("Acesso permitido"))), bloco(saida(texto("Acesso negado"))))),
         "Acesso permitido\n");
}

void testes_num_para() {
    // Exemplo 4.3 da proposta.
    Execucao soma_ate = caso(
        "proposta_repeticao",
        bloco(inteiro_v("contador", 1), inteiro_v("soma", 0),
              num_para(maior(id("contador"), inteiro(5)),
                       bloco(bota("soma", soma(id("soma"), id("contador"))), incrementa("contador"))),
              saida(id("soma"))),
        "15\n");
    contem("proposta_repeticao", soma_ate, "while not (v1_contador > 5):");

    caso("num_para_parada_inicial_verdadeira",
         bloco(num_para(papo(true), bloco(saida(texto("nunca")))), saida(texto("fim"))), "fim\n");

    caso("num_para_canso",
         bloco(inteiro_v("i", 0),
               num_para(papo(false), bloco(incrementa("i"), se(igual(id("i"), inteiro(3)), bloco(canso())))),
               saida(id("i"))),
         "3\n");

    caso("num_para_simbora",
         bloco(inteiro_v("i", 0),
               num_para(maior(id("i"), inteiro(3)),
                        bloco(incrementa("i"), se(igual(id("i"), inteiro(2)), bloco(simbora())), saida(id("i"))))),
         "1\n3\n4\n");
}

void testes_arrodeia() {
    Execucao simples = caso("arrodeia_basico",
                            bloco(inteiro_v("soma", 0),
                                  arrodeia(inteiro_v("i", 1), maior(id("i"), inteiro(4)), incrementa("i"),
                                           bloco(bota("soma", soma(id("soma"), id("i"))))),
                                  saida(id("soma"))),
                            "10\n");
    contem("arrodeia_basico", simples, "while not (v1_i > 4):");

    caso("arrodeia_parada_inicial_verdadeira",
         bloco(arrodeia(decl(TypeKind::Inteiro, "i", inteiro(9)), maior(id("i"), inteiro(4)), incrementa("i"),
                        bloco(saida(texto("nunca")))),
               saida(texto("fim"))),
         "fim\n");

    // A atualização roda quando simbora é executado; do contrário o laço não terminaria.
    Execucao continua = caso(
        "arrodeia_simbora_atualiza",
        bloco(arrodeia(inteiro_v("i", 1), maior(id("i"), inteiro(4)), incrementa("i"),
                       bloco(se(igual(resto(id("i"), inteiro(2)), inteiro(0)), bloco(simbora())), saida(id("i"))))),
        "1\n3\n");
    contem("arrodeia_simbora_atualiza", continua, "continue");

    caso("arrodeia_simbora_direto_no_corpo",
         bloco(inteiro_v("voltas", 0),
               arrodeia(inteiro_v("i", 1), maior(id("i"), inteiro(3)), incrementa("i"),
                        bloco(bota("voltas", soma(id("voltas"), inteiro(1))), simbora(), saida(texto("nunca")))),
               saida(id("voltas"))),
         "3\n");

    // canso não executa a atualização: i continua em 3, em vez de ir a 4.
    caso("arrodeia_canso",
         bloco(inteiro_v("i", 0),
               arrodeia(bota("i", inteiro(1)), maior(id("i"), inteiro(9)), incrementa("i"),
                        bloco(se(igual(id("i"), inteiro(3)), bloco(canso())))),
               saida(id("i"))),
         "3\n");

    caso("arrodeia_inicio_por_atribuicao",
         bloco(inteiro_v("i", 99), arrodeia(bota("i", inteiro(1)), maior(id("i"), inteiro(2)), incrementa("i"), bloco()),
               saida(id("i"))),
         "3\n");
}

void testes_aninhamento() {
    caso("canso_encerra_laco_interno",
         bloco(arrodeia(inteiro_v("i", 1), maior(id("i"), inteiro(2)), incrementa("i"),
                        bloco(arrodeia(inteiro_v("j", 1), maior(id("j"), inteiro(9)), incrementa("j"),
                                       bloco(se(igual(id("j"), inteiro(2)), bloco(canso())), saida(id("i"), id("j")))),
                              saida(texto("fim"), id("i"))))),
         "1 1\nfim 1\n2 1\nfim 2\n");

    // simbora no laço interno usa a atualização dele, e não a do externo.
    caso("simbora_usa_atualizacao_do_laco_interno",
         bloco(arrodeia(inteiro_v("i", 1), maior(id("i"), inteiro(2)), incrementa("i"),
                        bloco(arrodeia(inteiro_v("j", 1), maior(id("j"), inteiro(3)), incrementa("j"),
                                       bloco(se(igual(id("j"), inteiro(2)), bloco(simbora())), saida(id("i"), id("j"))))))),
         "1 1\n1 3\n2 1\n2 3\n");

    // simbora em num_para dentro de arrodeia não executa a atualização externa.
    Execucao misto = caso(
        "simbora_num_para_dentro_de_arrodeia",
        bloco(arrodeia(inteiro_v("i", 1), maior(id("i"), inteiro(2)), incrementa("i"),
                       bloco(inteiro_v("k", 0),
                             num_para(maior(id("k"), inteiro(1)),
                                      bloco(incrementa("k"), simbora(), saida(texto("nunca")))),
                             saida(id("i"), id("k"))))),
        "1 2\n2 2\n");
    contem("simbora_num_para_dentro_de_arrodeia", misto, "v1_k = v1_k + 1\n        continue");
    nao_contem("simbora_num_para_dentro_de_arrodeia", misto, "v1_i = v1_i + 1\n        continue");

    // simbora no arrodeia externo, depois do interno, ainda atualiza o externo.
    caso("simbora_em_arrodeia_contendo_num_para",
         bloco(arrodeia(inteiro_v("i", 1), maior(id("i"), inteiro(3)), incrementa("i"),
                        bloco(num_para(papo(false), bloco(canso())),
                              se(igual(id("i"), inteiro(2)), bloco(simbora())), saida(id("i"))))),
         "1\n3\n");

    caso("num_para_aninhados",
         bloco(inteiro_v("i", 0), inteiro_v("total", 0),
               num_para(igual(id("i"), inteiro(3)),
                        bloco(inteiro_v("j", 0),
                              num_para(igual(id("j"), inteiro(2)),
                                       bloco(bota("total", soma(id("total"), inteiro(1))), incrementa("j"))),
                              incrementa("i"))),
               saida(id("total"))),
         "6\n");

    rejeita("canso_fora_de_laco", [] { return bloco(canso()); });
    rejeita("simbora_fora_de_laco", [] { return bloco(se(papo(true), bloco(simbora()))); });
}

void testes_escopo() {
    // Mesmo nome em blocos independentes, com tipos diferentes.
    Execucao independentes = caso(
        "escopos_independentes",
        bloco(se(papo(true), bloco(inteiro_v("x", 1), saida(id("x")))),
              se(papo(true), bloco(decl(TypeKind::Resenha, "x", texto("oxe")), saida(id("x")))),
              se(papo(true), bloco(decl(TypeKind::Tronxo, "x", inteiro(2)), saida(id("x"))))),
        "1\noxe\n2,000\n");
    contem("escopos_independentes", independentes, "v1_x = 1");
    contem("escopos_independentes", independentes, "v2_x = \"oxe\"");
    contem("escopos_independentes", independentes, "v3_x = float(2)");

    // Após o primeiro laço, i pode ser redeclarado e a promoção usa o tipo novo.
    caso("arrodeia_consecutivos_reusam_nome",
         bloco(arrodeia(inteiro_v("i", 1), maior(id("i"), inteiro(1)), incrementa("i"), bloco(saida(id("i")))),
               arrodeia(decl(TypeKind::Tronxo, "i", inteiro(1)), maior(id("i"), inteiro(1)), bota("i", soma(id("i"), inteiro(1))),
                        bloco(saida(id("i"))))),
         "1\n1,000\n");

    // O nome do bloco encerrado deixa de ser visível para o gerador.
    rejeita("nome_de_bloco_encerrado", [] {
        return bloco(se(papo(true), bloco(inteiro_v("x", 1))), saida(id("x")));
    });
    rejeita("nome_de_arrodeia_encerrado", [] {
        return bloco(arrodeia(inteiro_v("i", 1), papo(true), incrementa("i"), bloco()), saida(id("i")));
    });

    // Variável declarada no corpo é refeita a cada volta.
    caso("declaracao_no_corpo_do_laco",
         bloco(arrodeia(inteiro_v("i", 1), maior(id("i"), inteiro(2)), incrementa("i"),
                        bloco(decl(TypeKind::Inteiro, "dobro", mul(id("i"), inteiro(2))), saida(id("dobro"))))),
         "2\n4\n");
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Uso: teste-codegen <python> <dir-runtime> <dir-temporario>\n";
        return 2;
    }
    python_exe = argv[1];
    dir_runtime = argv[2];
    dir_temporario = argv[3];
    std::filesystem::create_directories(dir_temporario);

    testes_expressoes();
    testes_condicionais();
    testes_num_para();
    testes_arrodeia();
    testes_aninhamento();
    testes_escopo();

    std::cout << casos - falhas << "/" << casos << " verificacoes de caso sem falha\n";
    if (falhas != 0) {
        std::cerr << falhas << " falha(s)\n";
        return 1;
    }
    return 0;
}
